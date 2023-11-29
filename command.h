#ifndef COMMAND_H
#define COMMAND_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "plugin.h"

/**
 * @brief Enumeration of command types.
 */
#define REGISTER_PLUGIN 0
#define UNREGISTE_RPLUGIN 1 
#define ADD_PLUGIN 2
#define DELETE_PLUGIN 3
#define ADD_FLOW_TO_QUEUE 4
#define DELETE_FLOW_FROM_QUEUE 5
#define ADD_QUEUE_TO_CORE 6
#define DELETE_QUEUE_FROM_CORE 7
#define DUMP_PLUGIN_RESULT 8

/**
 * @brief Represents a command structure.
 * 
 * This structure is used to define different types of commands and their corresponding arguments.
 * Each command type has a specific set of arguments defined in the union.
 */
typedef struct {
    int type; /**< The type of the command. */
    union {
        struct PluginInfo reg_plugin_arg; /**< Arguments for registering a plugin. */
        struct {
            int pluginid;
        } unreg_plugin_arg; /**< Arguments for unregistering a plugin. */
        struct {
            int pluginid;
            int coreid;
        } add_plugin_arg; /**< Arguments for adding a plugin. */
        struct {
            int pluginid;
            int coreid;
        } del_plugin_arg; /**< Arguments for deleting a plugin. */
        struct {
            unsigned short port_id;
            unsigned short rx_q;
            unsigned int src_ip;
            unsigned int src_mask;
            unsigned int dest_ip;
            unsigned int dest_mask;
        } add_flow_arg; /**< Arguments for adding a flow. */
        struct {
            unsigned short id;
        } del_flow_arg; /**< Arguments for deleting a flow. */
        struct {
            int queueid;
            int coreid;
        } add_queue_arg; /**< Arguments for adding a queue. */
        struct {
            int queueid;
            int coreid;
        } del_queue_arg; /**< Arguments for deleting a queue. */
        struct {
            int pluginid;
            int coreid;
            char filename[100];
        } dump_result_arg; /**< Arguments for dumping the result of a plugin. */
    } args; /**< The arguments of the command. */
} Command;


#endif