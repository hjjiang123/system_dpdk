#ifndef PLUGIN_H
#define PLUGIN_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include "capture.h"



struct PluginInterface
{
    size_t size;
    int id;
    const char * name;
    struct rte_flow *flow;
    void (*process)(struct rte_vlan_hdr *, struct rte_mbuf * );
};


#endif