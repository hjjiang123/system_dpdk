#include "../task.h"
#include "plugin.h"
#include <stdio.h> 
#include "PluginManager.h"
#include "FlowManager.h"
#include "../clienttest/client.h"
#include "dataplane.h"
// 解析子任务MSSubTask
MSSubTaskRuntimeNode * add_task_without_epoch(MSSubTask subtask){
    MSSubTaskRuntimeNode *trtnode = allocateMSSubTaskRuntime(subtask.id.id2);
    Command add_task_self_cmd={
        .type=ADD_SUBTASK_SELF,
        .args.add_task_self_arg.trtnode=trtnode
    };
    push_Command(add_task_self_cmd);
    return trtnode;
}
void add_task_flow(MSSubTaskRuntimeNode * trtnode){
    for (int i = 0; i < trtnode->trt.subtask.flow_num; i++){
        Command cmd5;
        cmd5.type = ADD_FLOW;
        cmd5.args.add_flow_arg.src_ip = trtnode->trt.subtask.flow[i].src_ip;
        cmd5.args.add_flow_arg.src_mask = trtnode->trt.subtask.flow[i].src_mask;
        cmd5.args.add_flow_arg.dest_ip = trtnode->trt.subtask.flow[i].dst_ip;
        cmd5.args.add_flow_arg.dest_mask = trtnode->trt.subtask.flow[i].dst_mask;
        cmd5.args.add_flow_arg.port_id = 0;
        cmd5.args.add_flow_arg.markid = trtnode->trt.pis[0].id.id2;
        sendCommand(cmd5);
    }
}

void delete_task_without_epoch(MSSubTaskRuntimeNode * trtnode){
    Command cmd2;
    cmd2.type = DELETE_SUBTASK;
    cmd2.args.del_task_arg.subtask_id = trtnode->trt.subtask.id.id2;
    cmd2.args.del_task_arg.coreid = trtnode->trt.subtask.core_id;
    sendCommand(cmd2);
    // 3. 删除流规则
}

void resolve_task(MSSubTask subtask){
    // 解析插件
    Command cmd1;
    cmd1.type = REGISTER_SUBTASK;
    cmd1.args.reg_task_arg = subtask;
    sendCommand(cmd1);
    if (subtask.type == 0){ // add
        if(subtask.times == 1){ //非周期性测量
            MSSubTaskRuntimeNode *trtnode = add_task_without_epoch(subtask);
            sleep(subtask.epoch);
            delete_task_without_epoch(trtnode);
        }
        else{ //周期性测量
            MSSubTaskRuntimeNode *trtnode[2];
            trtnode[0] = add_task_without_epoch(subtask);
            subtask.flip ^= 1;
            trtnode[1] = add_task_without_epoch(subtask);
            int t=0;
            while(t < subtask.times){
                // destroy_ipv4_flow_with_markid(0, subtask.pis[0].id.id2);
                // add_task_flow(subtask);
                enqueueMSSubTaskRuntimeNodeDump(trtnode[t%2]);
                t++;
                sleep(subtask.epoch);
            }
            // destroy_ipv4_flow_with_markid(0, subtask.pis[0].id.id2);
            delete_task_without_epoch(trtnode[0]);
            delete_task_without_epoch(trtnode[1]);
        }
    }
    Command cmdunreg;
    cmdunreg.type = REGISTER_SUBTASK;
    cmdunreg.args.unreg_task_arg.subtask_id = subtask.id.id2;
    sendCommand(cmdunreg);
    
}