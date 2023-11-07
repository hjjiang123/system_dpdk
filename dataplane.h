#ifndef DATAPLANE_H
#define DATAPLANE_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include "capture.h"
#include "plugin.h"
using namespace std;



class DataPlane
{
    vector<Plugin *> _handlers;
    vector<Plugin *> _handlers_updated;
    mutex _mt;
    int _record;

public:
    DataPlane()
    {
        _record = 0;
        Plugin *p = new PluginTest0(-1);
        _handlers.push_back(p);
        _handlers_updated.push_back(p);
    }

    void addPlugin(Plugin *newPlugin)
    {
        _mt.lock();
        _handlers_updated.push_back(newPlugin);
        _mt.unlock();
    }
    void run(int argc, char **argv)
    {
        int ret;
        unsigned int port_id = 0; // 需要抓取网卡的索引号
        struct rte_flow_error error;

        ret = rte_eal_init(argc, argv);
        if (ret < 0)
        {
            rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
        }

        // 获取可用网卡设备的数量
        int dev_ports = rte_eth_dev_count_avail();
        if (dev_ports == 0)
        {
            rte_exit(EXIT_FAILURE, "Error: No port available.\n");
        }

        // 创建内存池
        struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("mempool", NUM_MBUFS_DEFAULT, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
        if (mbuf_pool == NULL)
        {
            rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
        }

        // 初始化网卡设备
        ret = init_port(0, mbuf_pool);
        if (ret < 0)
        {
            rte_exit(EXIT_FAILURE, "Cannot init port %d\n", port_id);
        }

        force_quit = false;
        signal(SIGINT, signal_handler);

        while (!force_quit)
        {
            // 抓取数据包
            struct rte_mbuf *mbufs[DPDKCAP_CAPTURE_BURST_SIZE];

            //  rte_eth_rx_burst(port_id, queue_id, pkts, MAX_PKT_BURST), 返回抓取到的数据包数量
            int num_recvd = rte_eth_rx_burst(port_id, 0, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
            if (num_recvd == 0)
                continue;

            received_pkts += num_recvd;

            for (int i = 0; i < num_recvd; i++)
            {
                struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_ether_hdr *, 0);
                if (ntohs(eth_hdr->ether_type) == RTE_ETHER_TYPE_VLAN)
                {
                    struct rte_vlan_hdr *vlan_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_vlan_hdr *, sizeof(struct rte_ether_hdr));
                    if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV4)
                    {
                        
                        struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_ipv4_hdr *, (sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr)));
                        for (int j = 0; j < _handlers.size(); j++)
                        {
                            
                            _handlers[j]->process(ipv4_hdr,mbufs[i]);
                        }
                    }
                    else if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV6)
                    {
                        printf("ipv6\n");
                    }
                }
                else
                {
                    printf("不是RTE_ETHER_TYPE_VLAN数据包\n");
                }
                rte_pktmbuf_free(mbufs[i]);
            }
            _record++;
            // cout << "_record: " << _record << endl;
            if (_record == 10000)
            {
                _mt.lock();
                _handlers = _handlers_updated;
                _mt.unlock();
                _record = 0;
            }
        }
        
        // printf("共捕获: %d个数据包", received_pkts);

        rte_eal_mp_wait_lcore();
        rte_flow_flush(port_id, &error);
        rte_eth_dev_stop(port_id);
        rte_eth_dev_close(port_id);
        rte_eal_cleanup();
    }

    void updateTest()
    {
        int id = 0;
        while (true)
        {
            cout << "add " << id << endl;
            Plugin *p =new PluginTest1(id++);
            sleep(10);
            addPlugin(p);
            sleep(1000);
        }
    }
};

// int main()
// {
//     DataPlane dp;
//     thread t2(&DataPlane::updateTest, &dp);
//     dp.run();
//     t2.join();

//     return 0;
// }

#endif