#ifndef DATAPLANE_H
#define DATAPLANE_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>
#include "capture.h"
#include "plugin.h"
#include "flow_blocks.c"
#define MAX_CORE_NUMS 100

unsigned int _port_id = 0;                                                               // 需要抓取网卡的索引号
std::map<unsigned int, std::vector<std::shared_ptr<PluginInterface>>> _handlers;         // 运行在各个核的插件列表
std::map<unsigned int, std::vector<std::shared_ptr<PluginInterface>>> _handlers_updated; // 待更新的各个核的插件列表
std::map<int, std::shared_ptr<PluginInterface>> _plugins;                                // 插件库

int _nextId = 1; // 下一个可用的插件编号

std::mutex _mt[2][MAX_CORE_NUMS];  //_mt[0]用于_handlers更新，_mt[1]用于_core_queues更新
unsigned int _num_cores; // 获取可用的核心数量

std::vector<std::vector<int>> _core_queues(MAX_CORE_NUMS); //每核心监听的队列号
std::vector<std::vector<int>> _core_queues_updated(MAX_CORE_NUMS); //每核心监听的队列号


/************************************系统函数***************************************/
// 配置网卡设备号
void configurePort(unsigned int port_id)
{
    _port_id = port_id;
}

// 配置逻辑核数量
void configureNumCores(unsigned int numcores)
{
    _num_cores = numcores;
}

// 初始化dpdk环境
void init(int argc, char **argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    _num_cores = rte_lcore_count();
    if (_num_cores < 2)
        rte_exit(EXIT_FAILURE, "Insufficient cores\n");
}

// 每个核心的守护线程
int handle_packet_per_core(void *arg)
{
    unsigned int lcore_id = rte_lcore_id();
    std::cout << "startup lcore " << lcore_id << std::endl;
    struct rte_mbuf *mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    struct rte_ether_hdr *eth_hdr;
    int record = 0;
    // 接收报文并处理
    while (1)
    {
        for(int q=0;q<_core_queues[lcore_id].size();q++){
            int queue_id = _core_queues[lcore_id][q];
            const uint16_t nb_rx = rte_eth_rx_burst(_port_id, 0, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
            for (uint16_t i = 0; i < nb_rx; ++i)
            {
                eth_hdr = rte_pktmbuf_mtod(mbufs[i], struct rte_ether_hdr *);
                if (ntohs(eth_hdr->ether_type) == RTE_ETHER_TYPE_VLAN)
                {
                    struct rte_vlan_hdr *vlan_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_vlan_hdr *, sizeof(struct rte_ether_hdr));
                    for (int j = 0; j < _handlers[lcore_id].size(); j++)
                    {
                        _handlers[lcore_id][j]->process(vlan_hdr, mbufs[i]);
                    }
                }
                rte_pktmbuf_free(mbufs[i]);
            }
        }
        
        if (record++ == 10000000)
        {
            _mt[0][lcore_id].lock();
            _handlers[lcore_id] = _handlers_updated[lcore_id];
            _mt[0][lcore_id].unlock();
            record = 0;
            //修改queue_id
            _mt[1][lcore_id].lock();
            _core_queues[lcore_id] = _core_queues_updated[lcore_id];
            _mt[1][lcore_id].unlock();
        }
    }
}

// 运行主线程核部署子线程到每核心
void run()
{
    rte_flow_error error;
    for (int i = 0; i < _num_cores; i++)
    {
        int ret = rte_eal_remote_launch(handle_packet_per_core, NULL, i);
        if (ret != 0)
        {
            rte_panic("Cannot launch auxiliary thread\n");
        }
    }
    for (int i = 0; i < _num_cores; i++)
    {
        // 等待逻辑核心执行完毕
        int ret = rte_eal_wait_lcore(i);
        if (ret < 0)
        {
            rte_panic("Cannot wait for lcore %u\n", i);
        }
    }
    // rte_eal_mp_wait_lcore();
    // rte_flow_flush(_port_id, &error);
    rte_eth_dev_stop(_port_id);
    rte_eth_dev_close(_port_id);
    rte_eal_cleanup();
}

/************************************功能函数************************************/

// 注册插件
int registerPlugin(std::shared_ptr<PluginInterface> plugin)
{
    // 为插件分配编号并注册
    plugin->id = _nextId;
    _nextId++;
    _plugins[_nextId] = plugin;
    return plugin->id;
}

// 注销插件
void unregisterPlugin(std::shared_ptr<PluginInterface> plugin)
{
    // 在 plugins 列表中查找并移除指定的插件
    for (auto it = _plugins.begin(); it != _plugins.end();)
    {
        if (it->second->name == plugin->name)
        {
            it = _plugins.erase(it); // 删除当前元素，并返回下一个元素的迭代器
        }
        else
        {
            ++it;
        }
    }
}

// 添加插件到指定核心的待部署插件数组
void addPlugin(std::shared_ptr<PluginInterface> newPlugin, int coreid)
{
    _mt[0][coreid].lock();
    _handlers_updated[coreid].push_back(newPlugin);
    printf("_handlers_updated[%d]'s length = %d\n", coreid, _handlers_updated[coreid].size());
    _mt[0][coreid].unlock();
}

// 删除插件
void deletePlugin(std::shared_ptr<PluginInterface> plugin, int coreid)
{
    _mt[0][coreid].lock();
    // 在 plugins 列表中查找并移除指定的插make件
    auto it = std::find_if(_handlers_updated[coreid].begin(), _handlers_updated[coreid].end(), [&](const auto &p)
                           { return p->name == plugin->name; });
    if (it != _handlers_updated[coreid].end())
    {
        _handlers_updated[coreid].erase(it);
    }
    _mt[0][coreid].unlock();
}

// 添加流规则到指定队列
void addFlowToQueue(uint16_t port_id, uint16_t rx_q, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask)
{
    struct rte_flow_error error;
    rte_flow * flow = generate_ipv4_flow(port_id, rx_q,
				src_ip, src_mask,
				dest_ip, dest_mask, &error);
	/* >8 End of create flow and the flow rule. */
	if (!flow) {
		printf("Flow can't be created %d message: %s\n",
			error.type,
			error.message ? error.message : "(no stated reason)");
		rte_exit(EXIT_FAILURE, "error in creating flow");
	}
}

// 删除指定队列上的流规则
void deleteFlowQueue(uint16_t port_id,
		 struct rte_flow *flow)
{
    struct rte_flow_error error;
    int ret = rte_flow_destroy( port_id,flow,&error);
    if (ret) {
		printf("Flow can't be destroyed %d message: %s\n",
			error.type,
			error.message ? error.message : "(no stated reason)");
		rte_exit(EXIT_FAILURE, "error in destroying flow");
	}
}
// 添加核心处理流量的来源队列
void addQueueToCore(int queueid, int coreid){
    _mt[1][coreid].lock();
    _core_queues_updated[coreid].push_back(queueid);
    _mt[1][coreid].unlock();
}

// 删除核心处理流量的来源队列
void deleteQueueToCore(int queueid, int coreid){
    _mt[1][coreid].lock();
    auto it = std::find_if(_core_queues_updated[coreid].begin(), _core_queues_updated[coreid].end(), [&](const auto &p)
                           { return p == queueid; });
    if (it != _core_queues_updated[coreid].end())
    {
        _core_queues_updated[coreid].erase(it);
    }
    _mt[1][coreid].unlock();
}



#endif