/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include "dataplane.h"
#include "server.h"

// void processTest1(struct rte_vlan_hdr *vlan_hdr, struct rte_mbuf *pkt)
// {
// 	if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV4)
// 	{
// 		struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_ipv4_hdr *, (sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr)));
// 		if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
// 		{
// 			// 输出源端口和目的端口
// 			struct rte_tcp_hdr *tcp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_tcp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
// 			printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(tcp_hdr->src_port));
// 			printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(tcp_hdr->dst_port));
// 		}
// 		else if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
// 		{
// 			// 输出源端口和目的端口
// 			struct rte_udp_hdr *udp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_udp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
// 			printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(udp_hdr->src_port));
// 			printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(udp_hdr->dst_port));
// 		}
// 		else
// 		{
// 			printf("unknown protocol\n");
// 		}
// 	}
// 	else if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV6)
// 	{
// 		printf("ipv6\n");
// 	}
// }

// void processTest2(struct rte_vlan_hdr *vlan_hdr, struct rte_mbuf *pkt)
// {
// 	if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV4)
// 	{
// 		struct rte_ether_hdr *eth_hdr;
// 		struct rte_ipv4_hdr *ipv4_hdr;
// 		struct rte_tcp_hdr *tcp_hdr;

// 		eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
// 		ipv4_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
// 		tcp_hdr = (struct rte_tcp_hdr *)(ipv4_hdr + 1);

// 		// 判断是否为TCP报文
// 		if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
// 		{
// 			// 统计满足过滤条件的TCP报文数量
// 			printf("Received TCP packet on port %u\n", _port_id);
// 		}
// 	}
// 	else if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV6)
// 	{
// 		printf("ipv6\n");
// 	}
// }

// void updateTest()
// {
// 	std::shared_ptr<PluginInterface> p1(new PluginInterface{
// 		.size = 1024,
// 		.id = 2,
// 		.name = "testplugin1",
// 		.process = processTest1});
// 	std::shared_ptr<PluginInterface> p2(new PluginInterface{
// 		.size = 2048,
// 		.id = 3,
// 		.name = "testplugin2",
// 		.process = processTest2});
// 	sleep(5);
// 	addPlugin(p1, 0);
// 	sleep(5);
// 	addPlugin(p2, 0);
// }

int main(int argc, char **argv)
{

	init(argc, argv);
	// configureNumCores(5);
	int ret = init_port(_port_id);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE, "Cannot init port %d\n", _port_id);
	}
	
	runserver();

	// std::thread t2(updateTest);
	run();
	// t2.join();
	return 0;
}
