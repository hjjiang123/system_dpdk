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
#include <rte_flow.h>
#include <rte_per_lcore.h>
#include <rte_string_fns.h>
#include "config.h"

static const struct rte_eth_conf port_conf_default = {
  .rxmode = {
    .mq_mode = RTE_ETH_MQ_RX_NONE,
    .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
  }
};
extern unsigned int _port_id;
extern unsigned int _num_cores;
// Configure the NIC device number
void configurePort(unsigned int port_id);

// Configure the number of logical cores
void configureNumCores(unsigned int numcores);

// Initialize the dpdk environment
void init(int argc, char **argv);


// 初始化网卡端口
int init_port(unsigned int port_id);

#endif