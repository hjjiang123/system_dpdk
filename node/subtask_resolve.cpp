#include "subtask_resolve.h"
// 解析子任务MSSubTask
MSSubTaskRuntimeNode * add_task_without_epoch(MSSubTask *subtask){
    MSSubTaskRuntimeNode *trtnode = allocateMSSubTaskRuntime(subtask->id.id2);
    Command add_task_self_cmd={
        .type=ADD_SUBTASK_SELF,
        .args.add_task_self_arg.trtnode=trtnode
    };
    push_Command(add_task_self_cmd);
    return trtnode;
}
// void add_task_flow(MSSubTaskRuntimeNode * trtnode){
//     for (int i = 0; i < trtnode->trt.subtask->flow_num; i++){
//         Command cmd5;
//         cmd5.type = ADD_FLOW;
//         cmd5.args.add_flow_arg.src_ip = trtnode->trt.subtask->flow[i].src_ip;
//         cmd5.args.add_flow_arg.src_mask = trtnode->trt.subtask->flow[i].src_mask;
//         cmd5.args.add_flow_arg.dest_ip = trtnode->trt.subtask->flow[i].dst_ip;
//         cmd5.args.add_flow_arg.dest_mask = trtnode->trt.subtask->flow[i].dst_mask;
//         cmd5.args.add_flow_arg.port_id = 0;
//         cmd5.args.add_flow_arg.markid = trtnode->trt.pis[0].id.id2;
//         sendCommand(cmd5);
//     }
// }


void delete_task_without_epoch(MSSubTaskRuntimeNode * trtnode){
    Command cmd2;
    cmd2.type = DELETE_SUBTASK;
    cmd2.args.del_task_arg.subtask_id = trtnode->trt.subtask->id.id2;
    cmd2.args.del_task_arg.coreid = trtnode->trt.subtask->core_id;
    push_Command(cmd2);
    // 3. 删除流规则
}


void resolve_task(MSSubTask *subtask){
    
    if (subtask->type == 0){ // add
        // // 1.解析插件
        // registerSubTask(subtask);
        MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask->id.id2);
        // 2.加载插件
        if(subtask->times == 1){ //非周期性测量
            MSSubTaskRuntimeNode *trtnode = add_task_without_epoch(subtask);
            for(int i=0;i<subtask->epoch;i++){
                sleep(1);
                pthread_mutex_lock(&pi->run_mt);
                if(pi->run == false){
                    pthread_mutex_unlock(&pi->run_mt);
                    break;
                }
            }
            delete_task_without_epoch(trtnode);
            pthread_mutex_lock(&pi->run_mt);
            pi->run == false;
            pthread_mutex_unlock(&pi->run_mt);
        }
        else{ //周期性测量
            MSSubTaskRuntimeNode *trtnode[2];
            trtnode[0] = add_task_without_epoch(subtask);
            subtask->flip ^= 1;
            trtnode[1] = add_task_without_epoch(subtask);
            int t=0;
            while(t < subtask->times){
                // add_task_flow(subtask);
                t++;
                for(int i=0;i<subtask->epoch;i++){
                    sleep(1);
                    pthread_mutex_lock(&pi->run_mt);
                    if(pi->run == false){
                        pthread_mutex_unlock(&pi->run_mt);
                        delete_task_without_epoch(trtnode[0]);
                        delete_task_without_epoch(trtnode[1]);
                        return;
                    }
                }
                enqueueMSSubTaskRuntimeNodeDump(trtnode[t%2]);
                // destroy_ipv4_flow_with_markid(0, subtask->pis[0].id.id2);
            }
            // destroy_ipv4_flow_with_markid(0, subtask->pis[0].id.id2);
            delete_task_without_epoch(trtnode[0]);
            delete_task_without_epoch(trtnode[1]);
            pthread_mutex_lock(&pi->run_mt);
            pi->run == false;
            pthread_mutex_unlock(&pi->run_mt);
        }
    }else if (subtask->type == 1){ // delete
        unsigned int id = subtask->id.id2;
        MSSubTaskRuntimeNode *trtnode;
        getMSSubTaskRuntimeNode(id, trtnode);
        delete_task_without_epoch(trtnode);
        // 3. 删除流规则
    }
    
}

void set_task_stopped_forced(unsigned int subtaskid){
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtaskid);
    pthread_mutex_lock(&pi->run_mt);
    pi->run = false;
    pthread_mutex_unlock(&pi->run_mt);
}

bool get_task_running(unsigned int subtaskid){
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtaskid);
    pthread_mutex_lock(&pi->run_mt);
    bool run = pi->run;
    pthread_mutex_unlock(&pi->run_mt);
    return run;
}

void resolve_task_thread(MSSubTask *subtask) {
    std::thread t(resolve_task, subtask);
    // Wait for the thread to finish
    t.detach();
}



