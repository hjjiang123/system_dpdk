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

#include "FlowManager.h"
#include "PluginManager.h"
#include "PluginRuntimeManager.h"
#include "QueueManager.h"
#include "command.h"
#include "capture.h"




/************************************Global Variables***************************************/

struct lcoreCommandQueue {
    Command queue[SOCKET_QUEUE_SIZE];
    int front;
    int rear;
    std::mutex mt;
}; // Command queue for each core


/************************************Utility Functions************************************/

/**
 * Registers a plugin and returns its assigned ID.
 *
 * @param plugin A shared pointer to the PluginInfo object representing the plugin to be registered.
 * @return The ID assigned to the registered plugin, or -1 if the plugin failed to load.
 */
int registerPlugin(PluginInfo plugin);

/**
 * @brief Unregisters a plugin from the plugins list.
 *
 * This function searches for the specified plugin in the plugins list and removes it.
 *
 * @param plugin The shared pointer to the PluginInfo object representing the plugin to be unregistered.
 */
void unregisterPlugin(PluginInfo plugin);
/**
 * @brief Unregisters a plugin with the given plugin ID.
 * 
 * This function unloads the plugin associated with the given plugin ID.
 * 
 * @param pluginid The ID of the plugin to unregister.
 */
void unregisterPlugin(int pluginid);

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
void addPlugin(int pluginid, int coreid);

/**
 * @brief Deletes a plugin from the specified core.
 * 
 * This function deletes a plugin from the specified core. It releases the resources associated with the plugin, including freeing memory and releasing hash tables.
 * 
 * @param pluginid The ID of the plugin to be deleted.
 * @param coreid The ID of the core from which the plugin should be deleted.
 */
void deletePlugin(int pluginid, int coreid);

// Add a flow rule to the specified queue
int addFlowToQueue(uint16_t port_id, uint16_t rx_q, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask);

// Delete a flow rule from the specified queue
void deleteFlowFromQueue(int id);
// Add a source queue for core to process traffic
void addQueueToCore(int queueid, int coreid);

// Remove a source queue from core to process traffic
void deleteQueueFromCore(int queueid, int coreid);

void push_Command(Command c);

/************************************System Functions***************************************/
// Configure the NIC device number
void configurePort(unsigned int port_id);

// Configure the number of logical cores
void configureNumCores(unsigned int numcores);

// Initialize the dpdk environment
void init(int argc, char **argv);

// Daemon thread for each core
int handle_packet_per_core(void *arg);

// Main thread to deploy child threads to each core
void run();



#endif