#ifndef PLUGIN_RUNTIME_MANAGER_H
#define PLUGIN_RUNTIME_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <rte_flow.h>
#include <rte_hash.h>
#include <rte_hash_crc.h>
#include "plugin.h"
#include "config.h"
#include "PluginManager.h"

typedef struct PluginRuntimeNode
{
    PluginRuntime data;
    struct PluginRuntimeNode *next;
} PluginRuntimeNode;

typedef struct PluginRuntimeList
{
    PluginRuntimeNode *head;
    PluginRuntimeNode *tail;
} PluginRuntimeList;

typedef struct {
    PluginRuntimeNode *node;
    char *filename;
} PluginRuntimeDumps;

extern PluginRuntimeList _handlers_[MAX_CORE_NUMS]; // List of plugins running on each core


/**
 * @brief Enqueues a PluginRuntimeDumps node for processing.
 *
 * This function adds a PluginRuntimeDumps node to the processing queue.
 *
 * @param nodedump The PluginRuntimeDumps node to enqueue.
 * @return True if the node was successfully enqueued, false otherwise.
 */
bool enqueuePluginRuntimeNode(PluginRuntimeDumps *nodedump);

/**
 * @brief Initializes the plugin runtime list.
 * 
 * This function initializes the plugin runtime list by setting the head and tail pointers to NULL for each core.
 * 
 * @return void
 */
void initPluginRuntimeList();
bool getPluginRuntime(int pluginid, PluginRuntimeNode *node);

/**
 * @brief Adds a plugin runtime to the runtime manager.
 * 
 * This function allocates resources for the plugin, creates hash tables, 
 * retrieves the function pointer for the plugin, and creates a plugin runtime object.
 * The plugin runtime object is then added to the runtime manager.
 * 
 * @param pluginid The ID of the plugin.
 * @param coreid The ID of the core.
 */
void addPluginRuntime(int pluginid);



/**
 * @brief Pop a PluginRuntimeNode from the specified plugin and core.
 *
 * This function pops a PluginRuntimeNode from the plugin and core specified by the given pluginid and coreid.
 *
 * @param pluginid The ID of the plugin.
 * @param coreid The ID of the core.
 * @param node Pointer to the PluginRuntimeNode object to store the popped node.
 * @return true if a node was successfully popped, false otherwise.
 */
bool popPluginRuntime(int pluginid, PluginRuntimeNode *node);

/**
 * @brief Deletes a plugin runtime from the PluginRuntimeManager.
 * 
 * This function removes a plugin runtime identified by the given pluginid and coreid from the PluginRuntimeManager.
 * It releases the memory allocated for the plugin resources and hash tables associated with the plugin runtime.
 * 
 * @param pluginid The ID of the plugin runtime to be deleted.
 * @param coreid The ID of the core where the plugin runtime is running.
 */
void deletePluginRuntime(int pluginid);

/**
 * @brief Dump the PluginRuntimeNode information to a file.
 * 
 * This function dumps the information of a PluginRuntimeNode to a file specified by the filename parameter.
 * The information includes the plugin ID, core ID, and other relevant data of the node.
 * 
 * @param pluginid The ID of the plugin.
 * @param coreid The ID of the core.
 * @param filename The name of the file to dump the information to.
 * @param node The PluginRuntimeNode to be dumped.
 */
void dumpPluginRuntimeNode(int pluginid, char *filename, PluginRuntimeNode *node);
void dequeuePluginRuntimeNode();
// PluginRuntimeNode *findPluginRuntime(int pluginid, int coreid)
// {
//     PluginRuntimeNode *current = _handlers_[coreid].head;

//     while (current != NULL)
//     {
//         if (current->data.if == pluginid)
//         {
//             return current;
//         }

//         current = current->next;
//     }

//     return NULL;
// }
#endif