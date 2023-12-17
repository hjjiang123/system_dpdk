#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H
/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2017 Mellanox Technologies, Ltd
 */
#include <vector>
#include <rte_flow.h>
#include <stdlib.h>

#define MAX_PATTERN_NUM		3
#define MAX_ACTION_NUM		2

struct flow_id {
	int id;
	int port_id;
	unsigned int markid;
	struct rte_flow *flow;
};

struct flow_id*
generate_ipv4_flow(uint16_t port_id, uint32_t markid,
		uint32_t src_ip, uint32_t src_mask,
		uint32_t dest_ip, uint32_t dest_mask,
		struct rte_flow_error *error);

void destroy_ipv4_flow_with_id(int id);
void destroy_ipv4_flow_with_markid(int port_id,int mark_id);
#endif