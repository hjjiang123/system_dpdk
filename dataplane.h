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


unsigned int _port_id = 0; // 需要抓取网卡的索引号
std::map<unsigned int, std::vector<std::unique_ptr<PluginInterface>>> _handlers;  //运行在各个核的插件列表
std::map<unsigned int, std::vector<std::unique_ptr<PluginInterface>>> _handlers_updated; //待更新的各个核的插件列表
std::map<int, std::unique_ptr<PluginInterface>> _plugins;  //插件库

int _nextId = 1; // 下一个可用的插件编号

std::vector<std::mutex> _mt;
std::vector<int> _record;
unsigned int _num_cores; // 获取可用的核心数量


// 配置网卡设备号
void configurePort(unsigned int port_id){
    _port_id = port_id;
}

// 配置逻辑核数量 
void configureNumCores(unsigned int numcores){
    _num_cores = numcores;
    _record.resize(numcores);
    _mt.resize(numcores);
}

// 初始化dpdk环境
void init(int argc, char **argv){
    int ret = rte_eal_init(1, NULL);
    if (ret < 0) rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
    
    _num_cores = rte_lcore_count();
    if (_num_cores < 2) rte_exit(EXIT_FAILURE, "Insufficient cores\n");
}

// 每个核心的守护线程
int handle_packet_per_core(void* arg){
    unsigned int lcore_id = rte_lcore_id();
    std::cout<<"startup lcore "<<lcore_id<<std::endl;
    struct rte_mbuf* mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    struct rte_ether_hdr* eth_hdr;

    // 接收报文并处理
    while (1) {
        const uint16_t nb_rx = rte_eth_rx_burst(_port_id, 0, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
        for (uint16_t i = 0; i < nb_rx; ++i) {
            eth_hdr = rte_pktmbuf_mtod(mbufs[i], struct rte_ether_hdr*);
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
        _record[lcore_id]++;
        if (_record[lcore_id] == 10000)
        {
            _mt[lcore_id].lock();
            _handlers[lcore_id] = _handlers_updated[lcore_id];
            _mt[lcore_id].unlock();
            _record[lcore_id] = 0;
        }
    }
    
}

// 运行主线程核部署子线程到每核心
void run()
{
    rte_flow_error error;
    for(int i=0;i<_num_cores;i++){
        rte_eal_remote_launch(handle_packet_per_core,NULL,i);
    }
    rte_eal_mp_wait_lcore();
    rte_flow_flush(_port_id, &error);
    rte_eth_dev_stop(_port_id);
    rte_eth_dev_close(_port_id);
    rte_eal_cleanup();
}

// void updateTest()
// {
//     int id = 0;
//     while (true)
//     {
//         std::cout << "add " << id << std::endl;
//         PluginInterface *p = new PluginTest1(id++);
//         sleep(100);
//         addPlugin(p);
//         sleep(1000);
//     }
// }

// 注册插件
int registerPlugin(std::unique_ptr<PluginInterface> plugin)
{
    // 为插件分配编号并注册
    plugin->id = _nextId;
    _nextId++;
    _plugins[_nextId] = std::move(plugin);
    return plugin->id;
}

// 注销插件
void unregisterPlugin(std::unique_ptr<PluginInterface> plugin)
{
    // 在 plugins 列表中查找并移除指定的插件
    auto it = std::find_if(_plugins.begin(), _plugins.end(), [&](const auto &p)
                           { return p.second()->name == plugin->name; });
    if (it != _plugins.end())
    {
        _plugins.erase(it);
    }
}

// 添加插件到指定核心的待部署插件数组
void addPlugin(std::unique_ptr<PluginInterface> newPlugin, int coreid)
{
    _mt[coreid].lock();
    _handlers_updated[coreid].push_back(std::move(newPlugin));
    _mt[coreid].unlock();
}

// 删除插件
void deletePlugin(std::unique_ptr<PluginInterface> plugin, int coreid)
{
    // 在 plugins 列表中查找并移除指定的插件
    auto it = std::find_if(_handlers_updated[coreid].begin(), _handlers_updated[coreid].end(), [&](const auto &p)
                           { return p.second()->name == plugin->name; });
    if (it != _handlers_updated[coreid].end())
    {
        _handlers_updated[coreid].erase(it);
    }
}


#endif