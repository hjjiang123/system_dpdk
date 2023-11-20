#ifndef COMMAND_H
#define COMMAND_H

#include <rte_flow.h>
#include "config.h"
#include "plugin.h"

/**
 * @brief Enumeration of command types.
 */
typedef enum {
    REGISTER_PLUGIN,   
    UNREGISTE_RPLUGIN,   
    ADD_PLUGIN,  
    DELETE_PLUGIN,
    ADD_FLOW_TO_QUEUE, 
    DELETE_FLOW_FROM_QUEUE,
    ADD_QUEUE_TO_CORE,     
    DELETE_QUEUE_FROM_CORE,
} CommandType;

/**
 * @brief Represents a command structure.
 * 
 * This structure is used to define different types of commands and their corresponding arguments.
 * Each command type has a specific set of arguments defined in the union.
 */
typedef struct {
    CommandType type; /**< The type of the command. */
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
            uint16_t port_id;
            uint16_t rx_q;
            uint32_t src_ip;
            uint32_t src_mask;
            uint32_t dest_ip;
            uint32_t dest_mask;
        } add_flow_arg; /**< Arguments for adding a flow. */
        struct {
            uint16_t id;
        } del_flow_arg; /**< Arguments for deleting a flow. */
        struct {
            int queueid;
            int coreid;
        } add_queue_arg; /**< Arguments for adding a queue. */
        struct {
            int queueid;
            int coreid;
        } del_queue_arg; /**< Arguments for deleting a queue. */
    } args; /**< The arguments of the command. */
} Command;


/**
 * Serializes a Command object into a buffer.
 * 
 * @param command The Command object to be serialized.
 * @param buffer The buffer to store the serialized data.
 */
void serializeCommand(const Command* command, char* buffer) {
    memcpy(buffer, &(command->type), sizeof(CommandType));
    switch (command->type) {
        case REGISTER_PLUGIN:
            memcpy(buffer + sizeof(CommandType), &(command->args.reg_plugin_arg), sizeof(command->args.reg_plugin_arg));
            break;
        case UNREGISTE_RPLUGIN:
            memcpy(buffer + sizeof(CommandType), &(command->args.unreg_plugin_arg), sizeof(command->args.unreg_plugin_arg));
            break;
        case ADD_PLUGIN:
            memcpy(buffer + sizeof(CommandType), &(command->args.add_plugin_arg), sizeof(command->args.add_plugin_arg));
            break;
        case DELETE_PLUGIN:
            memcpy(buffer + sizeof(CommandType), &(command->args.del_plugin_arg), sizeof(command->args.del_plugin_arg));
            break;
        case ADD_FLOW_TO_QUEUE:
            memcpy(buffer + sizeof(CommandType), &(command->args.add_flow_arg), sizeof(command->args.add_flow_arg));
            break;
        case DELETE_FLOW_FROM_QUEUE:
            memcpy(buffer + sizeof(CommandType), &(command->args.del_flow_arg), sizeof(command->args.del_flow_arg));
            break;
        case ADD_QUEUE_TO_CORE:
            memcpy(buffer + sizeof(CommandType), &(command->args.add_queue_arg), sizeof(command->args.add_queue_arg));
            break;
        case DELETE_QUEUE_FROM_CORE:
            memcpy(buffer + sizeof(CommandType), &(command->args.del_queue_arg), sizeof(command->args.del_queue_arg));
            break;
        default:
            fprintf(stderr, "Invalid command type\n");
            exit(EXIT_FAILURE);
    }
}


/**
 * @brief Deserializes a command from a buffer.
 * 
 * This function deserializes a command from a buffer and populates the provided Command structure.
 * The buffer should contain the serialized command data.
 * 
 * @param buffer The buffer containing the serialized command data.
 * @param command Pointer to the Command structure to populate.
 */
void deserializeCommand(const char* buffer, Command* command) {
    memcpy(&(command->type), buffer, sizeof(CommandType));
    switch (command->type) {
        case REGISTER_PLUGIN:
            memcpy(&(command->args.reg_plugin_arg), buffer + sizeof(CommandType), sizeof(command->args.reg_plugin_arg));
            break;
        case UNREGISTE_RPLUGIN:
            memcpy(&(command->args.unreg_plugin_arg), buffer + sizeof(CommandType), sizeof(command->args.unreg_plugin_arg));
            break;
        case ADD_PLUGIN:
            memcpy(&(command->args.add_plugin_arg), buffer + sizeof(CommandType), sizeof(command->args.add_plugin_arg));
            break;
        case DELETE_PLUGIN:
            memcpy(&(command->args.del_plugin_arg), buffer + sizeof(CommandType), sizeof(command->args.del_plugin_arg));
            break;
        case ADD_FLOW_TO_QUEUE:
            memcpy(&(command->args.add_flow_arg), buffer + sizeof(CommandType), sizeof(command->args.add_flow_arg));
            break;
        case DELETE_FLOW_FROM_QUEUE:
            memcpy(&(command->args.del_flow_arg), buffer + sizeof(CommandType), sizeof(command->args.del_flow_arg));
            break;
        case ADD_QUEUE_TO_CORE:
            memcpy(&(command->args.del_queue_arg), buffer + sizeof(CommandType), sizeof(command->args.del_queue_arg));
            break;
        case DELETE_QUEUE_FROM_CORE:
            memcpy(&(command->args.del_queue_arg), buffer + sizeof(CommandType), sizeof(command->args.del_queue_arg));
            break;
        default:
            fprintf(stderr, "Invalid command type\n");
            exit(EXIT_FAILURE);
    }
}


#endif