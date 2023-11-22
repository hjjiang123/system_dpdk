#ifndef PLUGIN_RUNTIME_MANAGER_H
#define PLUGIN_RUNTIME_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include "plugin.h"
#include "dataplane.h"
#define MAX_CORE_NUMS 100

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


extern PluginRuntimeList _handlers_[MAX_CORE_NUMS]; // List of plugins running on each core



/**
 * @brief Initializes the plugin runtime list.
 * 
 * This function initializes the plugin runtime list by setting the head and tail pointers to NULL for each core.
 * 
 * @return void
 */
void initPluginRuntimeList();


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
void addPluginRuntime(int pluginid, int coreid);


/**
 * @brief Deletes a plugin runtime from the PluginRuntimeManager.
 * 
 * This function removes a plugin runtime identified by the given pluginid and coreid from the PluginRuntimeManager.
 * It releases the memory allocated for the plugin resources and hash tables associated with the plugin runtime.
 * 
 * @param pluginid The ID of the plugin runtime to be deleted.
 * @param coreid The ID of the core where the plugin runtime is running.
 */
void deletePluginRuntime(int pluginid, int coreid);



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