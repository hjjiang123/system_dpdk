#ifndef DATAPLANE_H
#define DATAPLANE_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <rte_hash.h>
#include <rte_hash_crc.h>

#include "capture.h"
#include "plugin.h"
#include "flow_blocks.c"
#include "PluginManager.h"
#include "command.h"

#define MAX_CORE_NUMS 100


/************************************Global Variables***************************************/
unsigned int _port_id = 0;          // Index of the NIC to capture

std::map<unsigned int, std::vector<std::shared_ptr<PluginRuntime>>> _handlers;  // List of plugins running on each core

struct lcoreCommandQueue {
    Command queue[SOCKET_QUEUE_SIZE];
    int front;
    int rear;
    std::mutex mt;
}; // Command queue for each core
std::map<unsigned int, lcoreCommandQueue> _command_queues;         // Command queues for each core,to add or delete plugin

PluginManager _PM;

int _nextId = 1; // Next available plugin ID

std::mutex _mt[2][MAX_CORE_NUMS]; //_mt[0] is used for _command_queues update, _mt[1] is used for _core_queues update
unsigned int _num_cores;          // Number of available cores

std::vector<std::vector<int>> _core_queues(MAX_CORE_NUMS);         // Queues listened by each core
std::vector<std::vector<int>> _core_queues_updated(MAX_CORE_NUMS); // Queues listened by each core to be updated

/************************************System Functions***************************************/
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

// Daemon thread for each core
int handle_packet_per_core(void *arg)
{
    unsigned int lcore_id = rte_lcore_id();
    std::cout << "startup lcore " << lcore_id << std::endl;
    struct rte_mbuf *mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    struct rte_ether_hdr *eth_hdr;
    int record = 0;
    // Receive and process packets
    while (1)
    {
        for (int q = 0; q < _core_queues[lcore_id].size(); q++)
        {
            int queue_id = _core_queues[lcore_id][q];
            const uint16_t nb_rx = rte_eth_rx_burst(_port_id, 0, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
            for (uint16_t i = 0; i < nb_rx; ++i)
            {
                eth_hdr = rte_pktmbuf_mtod(mbufs[i], struct rte_ether_hdr *);
                if (ntohs(eth_hdr->ether_type) == RTE_ETHER_TYPE_VLAN)
                {
                    struct rte_vlan_hdr *vlan_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_vlan_hdr *, sizeof(struct rte_ether_hdr));
                    for (int j = 0; j < _handlers[lcore_id].size(); j++)
                    {
                        _handlers[lcore_id][j]->func(vlan_hdr, mbufs[i], _handlers[lcore_id][j]->hash_table, _handlers[lcore_id][j]->res);
                    }
                }
                rte_pktmbuf_free(mbufs[i]);
            }
        }

        if (record++ == 10000000)
        {
            _command_queues[lcore_id].mt.lock();
            int k=_command_queues[lcore_id].front;
            while(k!=_command_queues[lcore_id].rear){
                switch (_command_queues[lcore_id].queue[k].type)
                {
                    case ADD_PLUGIN:
                        addPlugin(_command_queues[lcore_id].queue[k].args.add_plugin_arg.pluginid, _command_queues[lcore_id].queue[k].args.add_plugin_arg.coreid);
                        break;
                    case DELETE_PLUGIN:
                        deletePlugin(_command_queues[lcore_id].queue[k].args.del_plugin_arg.pluginid, _command_queues[lcore_id].queue[k].args.del_plugin_arg.coreid);
                        break;
                    default:
                        break;
                }
                k = (k+1) % SOCKET_QUEUE_SIZE;
            }
            _command_queues[lcore_id].front = k;
            _command_queues[lcore_id].mt.unlock();
            record = 0;
            // Update queue_id
            _mt[1][lcore_id].lock();
            _core_queues[lcore_id] = _core_queues_updated[lcore_id];
            _mt[1][lcore_id].unlock();
        }
    }
}

// Main thread to deploy child threads to each core
void run()
{
    rte_flow_error error;
    for (int i = 0; i < _num_cores; i++)
    {
        int ret = rte_eal_remote_launch(handle_packet_per_core, NULL, i);
        if (ret != 0)
        {
            rte_panic("Cannot launch auxiliary thread\n");
        }
    }
    for (int i = 0; i < _num_cores; i++)
    {
        // Wait for logical cores to finish execution
        int ret = rte_eal_wait_lcore(i);
        if (ret < 0)
        {
            rte_panic("Cannot wait for lcore %u\n", i);
        }
    }
    // rte_eal_mp_wait_lcore();
    // rte_flow_flush(_port_id, &error);
    rte_eth_dev_stop(_port_id);
    rte_eth_dev_close(_port_id);
    rte_eal_cleanup();
}

/************************************Utility Functions************************************/

/**
 * Registers a plugin and returns its assigned ID.
 *
 * @param plugin A shared pointer to the PluginInfo object representing the plugin to be registered.
 * @return The ID assigned to the registered plugin, or -1 if the plugin failed to load.
 */
int registerPlugin(std::shared_ptr<PluginInfo> plugin)
{
    return _PM.loadPlugin(*plugin);
}

/**
 * @brief Unregisters a plugin from the plugins list.
 *
 * This function searches for the specified plugin in the plugins list and removes it.
 *
 * @param plugin The shared pointer to the PluginInfo object representing the plugin to be unregistered.
 */
void unregisterPlugin(std::shared_ptr<PluginInfo> plugin)
{
    _PM.unloadPlugin(plugin->filename);
}

/**
 * @brief Unregisters a plugin with the given plugin ID.
 * 
 * This function unloads the plugin associated with the given plugin ID.
 * 
 * @param pluginid The ID of the plugin to unregister.
 */
void unregisterPlugin(int pluginid)
{
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    _PM.unloadPlugin(pi->filename);
}

/**
 * @brief Adds a plugin to the system.
 *
 * This function allocates resources for the plugin, creates a hash table,
 * retrieves the function pointer for the plugin, and creates a plugin runtime object.
 * The plugin is then added to the array of plugins to be deployed.
 *
 * @param pluginid The ID of the plugin.
 * @param coreid The ID of the core where the plugin will be deployed.
 */
void addPlugin(int pluginid, int coreid)
{
    // Get plugin information
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    // Allocate resources for the plugin
    Byte ****res = (Byte ****)malloc(pi->cnt_info.rownum * sizeof(Byte ***));
    for (int i = 0; i < pi->cnt_info.rownum; i++)
    {
        res[i] = (Byte ***)malloc(pi->cnt_info.bucketnum * sizeof(Byte **));
        for (int j = 0; j < pi->cnt_info.bucketnum; j++)
        {
            res[i][j] = (Byte **)malloc(pi->cnt_info.bucketsize * sizeof(Byte *));
            for (int k = 0; k < pi->cnt_info.bucketsize; k++)
            {
                res[i][j][k] = (Byte *)malloc(pi->cnt_info.countersize * sizeof(Byte));
            }
        }
    }
    // Create hash tables
    struct rte_hash *hash_table[pi->hash_info.hashnum];
    for (int i = 0; i < pi->hash_info.hashnum; i++)
    {
        char name[20];
        sprintf(name, "%d_%d", pi->id, i);
        struct rte_hash_parameters hash_params = {
            .name = name,
            .entries = pi->hash_info.entries,
            .key_len = pi->hash_info.key_len,
            .hash_func = rte_hash_crc,
            .hash_func_init_val = i,
        };
        hash_table[i] = rte_hash_create(&hash_params);
        if (hash_table[i] == NULL)
        {
            printf("Failed to create hash table %d\n", i);
            return;
        }
    }
    // Get the function pointer for the plugin
    PF myFunctionPtr = (PF)_PM.getFunction<PF>(pi->filename, pi->funcname);
    // Create a plugin runtime object
    std::shared_ptr<PluginRuntime> newPlugin = std::make_shared<PluginRuntime>(
        pluginid,
        res[pi->cnt_info.rownum][pi->cnt_info.bucketnum][pi->cnt_info.bucketsize][pi->cnt_info.countersize],
        hash_table[pi->hash_info.hashnum],
        myFunctionPtr);
    // Add to the array of plugins to be deployed
    _handlers[coreid].push_back(newPlugin);
}

void push_Command(Command c,std::map<unsigned int, lcoreCommandQueue> &command_queues){
    int lcore_id,plugin_id;
    if(c.type==ADD_PLUGIN){
        lcore_id = c.args.add_plugin_arg.coreid;
        plugin_id = c.args.add_plugin_arg.pluginid;
    }else if(c.type==DELETE_PLUGIN){
        lcore_id = c.args.del_plugin_arg.coreid;
        plugin_id = c.args.del_plugin_arg.pluginid;
    }else{
        printf("error CommandType\n");
        return;
    }
    command_queues[lcore_id].mt.lock();
    if((command_queues[lcore_id].rear+1)%SOCKET_QUEUE_SIZE==command_queues[lcore_id].front){
        printf("command queue is full\n");
    }else{
        command_queues[lcore_id].queue[command_queues[lcore_id].rear] = c;
        command_queues[lcore_id].rear = (command_queues[lcore_id].rear+1)%SOCKET_QUEUE_SIZE;
    }
    command_queues[lcore_id].mt.unlock();
    return;
}

/**
 * @brief Deletes a plugin from the specified core.
 * 
 * This function deletes a plugin from the specified core. It releases the resources associated with the plugin, including freeing memory and releasing hash tables.
 * 
 * @param pluginid The ID of the plugin to be deleted.
 * @param coreid The ID of the core from which the plugin should be deleted.
 */
void deletePlugin(int pluginid, int coreid)
{
    // Get plugin information
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    // Find and remove the specified plugin from the plugins list
    for (auto iter = _handlers[coreid].begin(); iter != _handlers[coreid].end(); iter++)
    {
        if ((*iter)->id == pluginid)
        {
            // Release resources
            for (int i = 0; i < pi->cnt_info.rownum; i++)
            {
                for (int j = 0; j < pi->cnt_info.bucketnum; j++)
                {
                    for (int k = 0; k < pi->cnt_info.bucketsize; k++)
                    {
                        free((void*)(*((*iter)->res))[i][j][k]);
                    }
                    free((void*)(*((*iter)->res))[i][j]);
                }
                free((void*)(*((*iter)->res))[i]);
            }
            free((void*)(*iter)->res);
            (*iter)->res = NULL;
            // Release hash tables
            for (int i = 0; i < pi->hash_info.hashnum; i++)
            {
                rte_hash_free((*iter)->hash_table[i]);
                (*iter)->hash_table[i] = NULL;
            }
            // Remove from the array of plugins to be deployed
            _handlers[coreid].erase(iter);
            break;
        }
    }
}

// Add a flow rule to the specified queue
int addFlowToQueue(uint16_t port_id, uint16_t rx_q, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask)
{
    struct rte_flow_error error;
    flow_id *flow = generate_ipv4_flow(port_id, rx_q,
                                       src_ip, src_mask,
                                       dest_ip, dest_mask, &error);
    /* >8 End of create flow and the flow rule. */
    if (!flow)
    {
        printf("Flow can't be created %d message: %s\n",
               error.type,
               error.message ? error.message : "(no stated reason)");
        rte_exit(EXIT_FAILURE, "error in creating flow");
    }
    return flow->id;
}

// Delete a flow rule from the specified queue
void deleteFlowFromQueue(int id)
{
    destroy_ipv4_flow_with_id(id);
}
// Add a source queue for core to process traffic
void addQueueToCore(int queueid, int coreid)
{
    _mt[1][coreid].lock();
    _core_queues_updated[coreid].push_back(queueid);
    _mt[1][coreid].unlock();
}

// Remove a source queue from core to process traffic
void deleteQueueFromCore(int queueid, int coreid)
{
    _mt[1][coreid].lock();
    auto it = std::find_if(_core_queues_updated[coreid].begin(), _core_queues_updated[coreid].end(), [&](const auto &p)
                           { return p == queueid; });
    if (it != _core_queues_updated[coreid].end())
    {
        _core_queues_updated[coreid].erase(it);
    }
    _mt[1][coreid].unlock();
}

#endif