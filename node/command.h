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
// #define REGISTER_PLUGIN 0
// #define UNREGISTE_RPLUGIN 1 
// #define ADD_PLUGIN 2
// #define DELETE_PLUGIN 3
// #define ADD_FLOW 4
// #define DELETE_FLOW 5
// #define ADD_QUEUE_TO_CORE 6
// #define DELETE_QUEUE_FROM_CORE 7
// #define DUMP_PLUGIN_RESULT 8

#define REGISTER_SUBTASK 0
#define UNREGISTE_SUBTASK 1 
#define ADD_SUBTASK 2
#define ADD_SUBTASK_SELF 201
#define DELETE_SUBTASK 3
#define ADD_FLOW 4
#define DELETE_FLOW 5
// #define ADD_QUEUE_TO_CORE 6
// #define DELETE_QUEUE_FROM_CORE 7
#define DUMP_SUBTASK_RESULT 6
#define MONITOR_ALL_SUBTASKS 7

/**
 * @brief Represents a command structure.
 * 
 * This structure is used to define different types of commands and their corresponding arguments.
 * Each command type has a specific set of arguments defined in the union.
 */
typedef struct {
    int type; /**< The type of the command. */
    union {
        struct MSSubTask reg_task_arg; /**< Arguments for registering a plugin. */
        struct unreg_task_arg {
            unsigned int subtask_id;
        } unreg_task_arg; /**< Arguments for unregistering a plugin. */
        struct add_task_arg{
            unsigned int subtask_id;
            int coreid;
        } add_task_arg; /**< Arguments for adding a task. */
        struct del_task_arg{
            unsigned int subtask_id;
            int coreid;
        } del_task_arg; /**< Arguments for deleting a task. */
        struct add_flow_arg{
            unsigned short port_id;
            unsigned int markid;
            unsigned int priority;
            unsigned int src_ip;
            unsigned int src_mask;
            unsigned int dest_ip;
            unsigned int dest_mask;
        } add_flow_arg; /**< Arguments for adding a flow. */
        struct del_flow_arg{
            unsigned short id;
        } del_flow_arg; /**< Arguments for deleting a flow. */
        struct add_task_self_arg{
            MSSubTaskRuntimeNode *trtnode;
        } add_task_self_arg;
        struct monitor_task_arg{
            int num;
            unsigned int subtask_ids[64];
        } monitor_task_arg;
    } args; /**< The arguments of the command. */
} Command;


#endif