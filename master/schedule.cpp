#include "schedule.h"

std::map<unsigned int,MSTask> tasks;  //维护当前已加载的任务

std::map<unsigned int,std::map<unsigned int,MSSubTask>> subtasks; //维护当前已调度的子任务

std::queue<MSTask> taskQueue; //维护待加载的任务
std::mutex queueMutex; //维护待加载的任务的互斥锁

//注册MSTask并分配task_id
void registerMSTask(MSTask& task) {
    //分配一个不重复的task_id
    int task_id = 0;
    while (tasks.find(task_id) != tasks.end()) {
        task_id++;
    }
    //为MSTask赋值task_id属性
    task.task_id = task_id;
    tasks[task_id] = task;
    subtasks[task_id] = std::map<unsigned int,MSSubTask>();
}

//注销MSTask并释放task_id
void unregisterMSTask(MSTask& task) {
    subtasks.erase(task.task_id);
    //释放task_id
    tasks.erase(task.task_id);
}


//注册子任务
bool registerSubTask(unsigned int task_id, MSSubTask& subtask) {
    if(subtasks.find(task_id) == subtasks.end()){ //任务不存在
        return false;
    }
    //注册子任务
    //分配一个不重复的subtask_id
    unsigned int subtask_id = 0;
    while (subtasks[task_id].find(subtask_id) != subtasks[task_id].end()) {
        subtask_id++;
    }
    if(subtask_id > 1023){ //子任务号超出范围
        return false;
    }
    subtask.id.id1.task_id = task_id;
    subtask.id.id1.subtask_id = subtask_id;
    subtasks[task_id][subtask_id] = subtask;
    return true;
}
//注销子任务
void unregisterSubTask(MSSubTask& subtask) {
    int task_id = subtask.id.id1.task_id;
    int subtask_id = subtask.id.id1.subtask_id;
    //注销子任务
    subtasks[task_id].erase(subtask.id.id1.subtask_id);
}

// 入队MSTask
void enqueueMSTask(const MSTask& task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(task);
}

// 出队MSTask
MSTask dequeueMSTask() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (taskQueue.empty()) {
        // 队列为空，返回一个默认构造的MSTask对象
        return MSTask();
    }
    MSTask task = taskQueue.front();
    taskQueue.pop();
    return task;
}

bool scheduleMSTask_once(){
    MSTask task = dequeueMSTask();
    if(task.task_id == 0){ //队列为空
        return false;
    }
    registerMSTask(task);
    // generateSubTask(task);
    return true;    
}
MSFlowEntry findFlowIntersection_once(MSFlowEntry e1, MSFlowEntry e2){//e1&e2
    MSFlowEntry e;
    return e;
};
MSFlowEntry findFlowDifference(MSFlowEntry e1, MSFlowEntry e2){ //e1-e2
    MSFlowEntry diff;
    // Perform set difference operation on the flow entries
    // and store the result in 'diff'
    // ...
    return diff;
};
//查找给定任务与所有已调度子任务的流规则的重合和非重合部分
std::vector<FlowEntryDupInfo> findFlowIntersection(MSTask& task) {
    //查找所有已调度子任务的流规则
    std::vector<FlowEntryDupInfo> dupflows;
    for(int fi=0;fi<task.flow_num;fi++){
        MSFlowEntry& flow = task.flow[fi];
        for(auto taskiter = subtasks.begin(); taskiter != subtasks.end(); taskiter++){
            for(auto subtaskiter = taskiter->second.begin(); subtaskiter != taskiter->second.end(); subtaskiter++){
                MSSubTask& subtask = subtaskiter->second;
                for(int i = 0; i < subtask.flow_num; i++){
                    MSFlowEntry e = findFlowIntersection_once(flow,subtask.flow[i]);
                    if(e.direction != -1){
                        FlowEntryDupInfo dupflow;
                        dupflow.task_id = taskiter->first;
                        dupflow.subtask_id = subtaskiter->first;
                        dupflow.flow_index = i;
                        dupflows.push_back(dupflow);
                    }
                }
            }
        }
    }
    
    return dupflows;
}
//调度任务生成子任务
void generateSubTask(MSTask& task) {
    //查找所有已调度子任务的流规则

}

//调度任务生成子任务