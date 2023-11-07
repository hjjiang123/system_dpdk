#ifndef PLUGIN_H
#define PLUGIN_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include "capture.h"

using namespace std;

class Plugin
{
    size_t _size;
    int _id;

public:
    Plugin(int id)
    {
        _id = id;
        _size = 0;
    }
    Plugin(const Plugin &p)
    {
        _id = p.getID();
        _size = p.getSize();
    }
    void setSize(size_t size)
    {
        _size = size;
    }
    int getID() const
    {
        return _id;
    }
    int getSize() const
    {
        return _size;
    }
    Plugin &operator=(const Plugin &p)
    {
        _id = p.getID();
        _size = p.getSize();
        return *this;
    }
    virtual void process(struct rte_ipv4_hdr *ipv4_hdr, struct rte_mbuf * mbuf)=0;
};


class PluginTest0: public Plugin
{
public:
    PluginTest0(int id):Plugin(id){}
    void process(struct rte_ipv4_hdr *ipv4_hdr, struct rte_mbuf * mbuf) override
    {
    }
};

class PluginTest1: public Plugin
{
public:
    PluginTest1(int id):Plugin(id){}
    void process(struct rte_ipv4_hdr *ipv4_hdr, struct rte_mbuf * mbuf) override
    {
        if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
        {
            // 输出源端口和目的端口
            struct rte_tcp_hdr *tcp_hdr = rte_pktmbuf_mtod_offset(mbuf, struct rte_tcp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
            printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(tcp_hdr->src_port));
            printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(tcp_hdr->dst_port));
        }
        else if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
        {
            // 输出源端口和目的端口
            struct rte_udp_hdr *udp_hdr = rte_pktmbuf_mtod_offset(mbuf, struct rte_udp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
            printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(udp_hdr->src_port));
            printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(udp_hdr->dst_port));
        }
        else
        {
            printf("unknown protocol\n");
        }
    }
};

// int main()
// {
//     DataPlane dp;
//     thread t2(&DataPlane::update, &dp);
//     dp.run();
//     t2.join();

//     return 0;
// }

#endif