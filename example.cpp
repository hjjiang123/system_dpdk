#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_flow.h>
#define RX_RING_SIZE 128
#define TX_RING_SIZE 512
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static const struct rte_eth_conf port_conf_default = {
    .rxmode = {
        .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
    },
};

static const struct rte_flow_attr ingress = {
    .ingress = 1,
};

static const struct rte_flow_action_port_id port_id_action = {
    .id = 1,
};

static const struct rte_flow_action_jump jump = {
    .group = 1,
};

static const struct rte_flow_action actions[] = {
    {
        .type = RTE_FLOW_ACTION_TYPE_PORT_ID,
        .conf = &port_id_action,
    },
    {
        .type = RTE_FLOW_ACTION_TYPE_JUMP,
        .conf = &jump,
    },
    {
        .type = RTE_FLOW_ACTION_TYPE_END,
    },
};

static const struct rte_flow_item_ipv4 ipv4_pattern = {
    .hdr = {
        .next_proto_id = IPPROTO_TCP,
    },
};

static const struct rte_flow_item_eth eth_pattern = {
    .type = RTE_BE16(RTE_ETHER_TYPE_IPV4),
};

static const struct rte_flow_item items[] = {
    {
        .type = RTE_FLOW_ITEM_TYPE_ETH,
        .spec = &eth_pattern,
        .mask = NULL,
    },
    {
        .type = RTE_FLOW_ITEM_TYPE_IPV4,
        .spec = &ipv4_pattern,
        .mask = NULL,
    },
    {
        .type = RTE_FLOW_ITEM_TYPE_END,
    },
};

int main(int argc, char *argv[]) {
    int ret;
    unsigned nb_ports;
    uint16_t port_id;
    struct rte_mempool *mbuf_pool;
    struct rte_eth_conf port_conf = port_conf_default;
    struct rte_flow_error error;
    struct rte_flow_attr attr = ingress;
    struct rte_flow_item pattern[sizeof(items) / sizeof(items[0])];
    struct rte_flow *flow;

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: EAL initialization failed!\n");

    nb_ports = rte_eth_dev_count_avail();
    if (nb_ports == 0)
        rte_exit(EXIT_FAILURE, "Error: No Ethernet ports found!\n");

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Error: Failed to create mbuf pool!\n");

    port_id = 0; // Assuming the first port is used
    
    ret = rte_eth_dev_configure(port_id, 1, 1, &port_conf);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: Failed to configure Ethernet port!\n");

    ret = rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: Failed to set up RX queue!\n");

    ret = rte_eth_tx_queue_setup(port_id, 0, TX_RING_SIZE, rte_eth_dev_socket_id(port_id), NULL);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: Failed to set up TX queue!\n");

    ret = rte_eth_dev_start(port_id);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: Failed to start Ethernet port!\n");

    ret = rte_flow_validate(port_id, &attr, pattern, actions, &error);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error: Failed to validate flow rule! Message: %s\n", error.message);

    flow = rte_flow_create(port_id, &attr, pattern, actions, &error);
    if (flow == NULL)
        rte_exit(EXIT_FAILURE, "Error: Failed to create flow rule! Message: %s\n", error.message);

    printf("Flow rule created successfully!\n");

    rte_flow_apply(flow, port_id);

    // Wait for some time to allow the flow rule to take effect
    sleep(5);

    rte_flow_flush(port_id, &error);

    rte_eth_dev_stop(port_id);
    rte_eth_dev_close(port_id);

    return 0;
}