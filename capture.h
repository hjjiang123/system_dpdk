#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <signal.h>
#include <rte_common.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>
#include <rte_launch.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_per_lcore.h>
#include <rte_string_fns.h>


#define RX_DESC_DEFAULT 512
#define NUM_MBUFS_DEFAULT 8192
#define MBUF_CACHE_SIZE 256
#define DPDKCAP_CAPTURE_BURST_SIZE 256

int received_pkts = 0;
static volatile bool force_quit;
static const struct rte_eth_conf port_conf_default = {
  .rxmode = {
    .mq_mode = RTE_ETH_MQ_RX_NONE,
    .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
  }
};

static void signal_handler(int sigquit) {
	if (sigquit == SIGINT) {
		int aaa, bbb;
		if(scanf("%d%d", &aaa, &bbb) == 2) {
			int ccc = aaa+bbb;
			printf("准备停止le%d\n", ccc);
			printf("共捕获: %d个数据包\n", received_pkts);
			force_quit = true;
		}
		else
			force_quit = true;
	}
}

// 初始化网卡端口
int init_port(unsigned int port_id, struct rte_mempool *mbuf_pool) {
	int ret;

	// 判断该网卡端口是否存在
	ret = rte_eth_dev_is_valid_port(port_id);
	if (ret == 0) {
		rte_exit(EXIT_FAILURE, "Invalid port_id %d\n", port_id);
	}

	// 设置网卡端口的配置
	ret = rte_eth_dev_configure(port_id, 1, 0, &port_conf_default);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%d\n", ret, port_id);
		return ret;
	}

	// // 启动队列
	// ret = rte_eth_rx_queue_setup(port_id, 0, RX_DESC_DEFAULT, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
	// if (ret < 0) {
	// 	rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%d\n", ret, port_id);
	// 	return ret;
	// }

	// 开启混杂模式
	rte_eth_promiscuous_enable(port_id);

	// 启动网卡端口
	ret = rte_eth_dev_start(port_id);

	return 0;
}

#endif