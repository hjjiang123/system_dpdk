#include "capture.h"

unsigned int _port_id = 0;          // Index of the NIC to capture
unsigned int _num_cores;          // Number of available cores
// Configure the NIC device number
void configurePort(unsigned int port_id)
{
    _port_id = port_id;
}

// Configure the number of logical cores
void configureNumCores(unsigned int numcores)
{
    _num_cores = numcores;
}

// Initialize the dpdk environment
void init(int argc, char **argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    _num_cores = rte_lcore_count();
    if (_num_cores < 2)
        rte_exit(EXIT_FAILURE, "Insufficient cores\n");
}

// 初始化网卡端口
int init_port(unsigned int port_id)
{
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
	// 创建内存池
	struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("mempool", NUM_MBUFS_DEFAULT, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL) {
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
	}
	// 启动队列
	ret = rte_eth_rx_queue_setup(port_id, 0, RX_DESC_DEFAULT, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%d\n", ret, port_id);
		return ret;
	}

	// 开启混杂模式
	rte_eth_promiscuous_enable(port_id);

	// 启动网卡端口
	ret = rte_eth_dev_start(port_id);

	return 0;
}