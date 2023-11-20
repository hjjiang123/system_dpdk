#ifndef COMMAND_H
#define COMMAND_H

#include <rte_flow.h>
#include "config.h"
#include "plugin.h"

// 定义指令类型
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

// 定义指令结构体
typedef struct {
    CommandType type;
    union {
        struct PluginInfo reg_plugin_arg; // 注册插件
        struct {
            int pluginid;
        } unreg_plugin_arg;// 取消注册插件
        struct {
            int pluginid;
            int coreid;
        } add_plugin_arg;// 添加插件
        struct {
            int pluginid;
            int coreid;
        } del_plugin_arg;// 删除插件
        struct {
            uint16_t port_id;
            uint16_t rx_q;
            uint32_t src_ip;
            uint32_t src_mask;
            uint32_t dest_ip;
            uint32_t dest_mask;
        } add_flow_arg;// 添加流
        struct {
            uint16_t id;
        } del_flow_arg;// 删除流
        struct {
            int queueid;
            int coreid;
        } add_queue_arg;// 添加队列
        struct {
            int queueid;
            int coreid;
        } del_queue_arg;// 删除队列
    } args;
} Command;
// command.h文件中Command结构体怎样序列化成字节流，要求反序列化后生成的Command和原先一模一样
// 将指令结构体序列化为字节流
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

// 将字节流反序列化为指令结构体
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