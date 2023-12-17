#include "../task.h"
#include "plugin.h"
#include <stdio.h> 
#include "PluginManager.h"
#include "FlowManager.h"
#include "../clienttest/client.h"
// 解析子任务MSSubTask

void add_task_without_epoch(MSSubTask subtask){
    for (int i = 0; i < subtask.pi_num; i++){
        Command cmd3;
        cmd3.type = ADD_PLUGIN;
        cmd3.args.add_plugin_arg.pluginid = subtask.pis[i].id.id2;
        cmd3.args.add_plugin_arg.coreid = subtask.pis[i].id.id1.core_id;
        sendCommand(cmd3);
    }
}
void add_task_flow(MSSubTask subtask){
    for (int i = 0; i < subtask.flow_num; i++){
        Command cmd5;
        cmd5.type = ADD_FLOW;
        cmd5.args.add_flow_arg.src_ip = subtask.flow[i].src_ip;
        cmd5.args.add_flow_arg.src_mask = subtask.flow[i].src_mask;
        cmd5.args.add_flow_arg.dest_ip = subtask.flow[i].dst_ip;
        cmd5.args.add_flow_arg.dest_mask = subtask.flow[i].dst_mask;
        cmd5.args.add_flow_arg.port_id = 0;
        cmd5.args.add_flow_arg.markid = subtask.pis[0].id.id2;
        sendCommand(cmd5);
    }
}

void delete_task_without_epoch(MSSubTask subtask){
    destroy_ipv4_flow_with_markid(0, subtask.pis[0].id.id2); 
    for (int i = 0; i < subtask.pi_num; i++){
        Command cmd3;
        cmd3.type = DELETE_PLUGIN;
        cmd3.args.del_plugin_arg.pluginid = subtask.pis[i].id.id2;
        cmd3.args.del_plugin_arg.coreid = subtask.pis[i].id.id1.core_id;
        sendCommand(cmd3);
    }
}

void resolve_task(MSSubTask subtask){
    printf("resolve_task %d\n",subtask.pis[0].id.id2);
    // 2. 解析插件
    for (int i = 0; i < subtask.pi_num; i++){
        Command cmd1;
        cmd1.type = REGISTER_PLUGIN;
        cmd1.args.reg_plugin_arg = subtask.pis[i];
        sendCommand(cmd1);
    }
    if (subtask.type == 0){ // add
        if(subtask.times == 1){ //非周期性测量
            add_task_without_epoch(subtask);
            sleep(subtask.epoch);
            delete_task_without_epoch(subtask);
        }
        else{ //周期性测量
            add_task_without_epoch(subtask);
            for(int i=0;i<subtask.pi_num;i++){
                    subtask.pis[i].id.id1.flip ^= 1;
            }
            add_task_without_epoch(subtask);
            int t=0;
            while(t < subtask.times){
                destroy_ipv4_flow_with_markid(0, subtask.pis[0].id.id2);
                for(int i=0;i<subtask.pi_num;i++){
                    subtask.pis[i].id.id1.flip ^= 1;
                }
                add_task_flow(subtask);
                t++;
                sleep(subtask.epoch);
            }
            destroy_ipv4_flow_with_markid(0, subtask.pis[0].id.id2);
            delete_task_without_epoch(subtask);
            for(int i=0;i<subtask.pi_num;i++){
                    subtask.pis[i].id.id1.flip ^= 1;
            }
            delete_task_without_epoch(subtask);
        }
    }
    
}