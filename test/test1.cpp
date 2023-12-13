#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <unistd.h> // Add this line to include the <unistd.h> header
#define RX_RING_SIZE 128
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#include <rte_ethdev.h>

int main(int argc, char *argv[]) {
    int ret;
    uint16_t port_id;
    struct rte_mempool *mbuf_pool;
    struct rte_eth_dev_info dev_info;
    struct rte_eth_stats stats;

    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error: rte_eal_init failed\n");
    }

    argc -= ret;
    argv += ret;

    ret = rte_eth_dev_count_avail();
    if (ret == 0) {
        rte_exit(EXIT_FAILURE, "Error: No Ethernet ports found\n");
    }

    port_id = 0;

    ret = rte_eth_dev_configure(port_id, 1, 1, NULL);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error: Cannot configure port %u\n", port_id);
    }

    ret = rte_eth_dev_info_get(port_id, &dev_info);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error: Cannot get device info for port %u\n", port_id);
    }

    mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (mbuf_pool == NULL) {
        rte_exit(EXIT_FAILURE, "Error: Cannot create mbuf pool\n");
    }

    ret = rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
    

    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error: Cannot setup RX queue for port %u\n", port_id);
    }

    ret = rte_eth_dev_start(port_id);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error: Cannot start port %u\n", port_id);
    }

    printf("Capturing packets for 3 seconds...\n");
    sleep(3);

    rte_eth_stats_get(port_id, &stats);
    printf("Packets captured: %" PRIu64 "\n", stats.ipackets);

    rte_eth_dev_stop(port_id);
    rte_eth_dev_close(port_id);

    return 0;
}
