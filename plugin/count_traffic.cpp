#include <rte_ethdev.h>
#include <rte_hash.h>
#include <rte_hash_crc.h>
extern "C"{
void process(struct rte_mbuf *pkt, char ****res)
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
		if (ipv4_hdr->dst_addr<0xffffffff && ipv4_hdr->dst_addr>0x00000000){
			unsigned int key = ipv4_hdr->src_addr;
			unsigned int value = 1;
			for(int i=0;i<8;i++){
				uint32_t hash_val = rte_hash_crc_4byte(ipv4_hdr->src_addr, i)>>20;
				res[0][hash_val][0][0] = 1;
			}
		}
	}
	else if (ntohs(vlan_hdr->eth_proto) == RTE_ETHER_TYPE_IPV6)
	{
		printf("ipv6\n");
	}
}
}
