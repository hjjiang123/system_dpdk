#include <rte_ethdev.h>
extern "C"{
void process(struct rte_mbuf *pkt,  char ****res)
{
	struct rte_vlan_hdr *vlan_hdr;
	struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
	if (ntohs(eth_hdr->ether_type) == RTE_ETHER_TYPE_VLAN)
	{
		vlan_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_vlan_hdr *, sizeof(struct rte_ether_hdr));
	}
	if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV4)
	{
		struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_ipv4_hdr *, (sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr)));
		if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
		{
			// 输出源端口和目的端口
			struct rte_tcp_hdr *tcp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_tcp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
			printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(tcp_hdr->src_port));
			printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(tcp_hdr->dst_port));
		}
		else if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
		{
			// 输出源端口和目的端口
			struct rte_udp_hdr *udp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_udp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr) + sizeof(struct rte_ipv4_hdr));
			printf("src: %d.%d.%d.%d:%d -> ", ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF, (ipv4_hdr->src_addr >> 16) & 0xFF, (ipv4_hdr->src_addr >> 24) & 0xFF, ntohs(udp_hdr->src_port));
			printf("dst: %d.%d.%d.%d:%d\n", ipv4_hdr->dst_addr & 0xFF, (ipv4_hdr->dst_addr >> 8) & 0xFF, (ipv4_hdr->dst_addr >> 16) & 0xFF, (ipv4_hdr->dst_addr >> 24) & 0xFF, ntohs(udp_hdr->dst_port));
		}
		else
		{
			printf("unknown protocol\n");
		}
	}
	else if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV6)
	{
		printf("ipv6\n");
	}
}
}
