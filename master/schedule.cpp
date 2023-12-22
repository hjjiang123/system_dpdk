#include "schedule.h"

typedef std::map<unsigned int, std::map<unsigned int,MSSubTask> > SubTaskMap; //任务号 - 子任务集合

const unsigned int _resource[10][40]=
                {100,100,100,100,100,100,100,100,100,100,};
const unsigned int _host_num = 10;
const unsigned int _core_num = 40*10;

std::map<unsigned int,MSTask> tasks;  //维护当前已加载的任务

SubTaskMap subtasks; //维护当前已调度的子任务

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

std::vector<MSFlowEntry> splitFlowEntry_with_subtask( MSFlowEntry fe, unsigned int i,
                                    MSSubTask subtask){//将flowentry分割成多个flowentry
    std::vector<MSFlowEntry> flowentries;
    MSFlowEntry one_entry = subtask.flow[i];
    if(i==subtask.flow_num-1){ //最后一个flow
        if(hasFlowIntersection(fe,one_entry)){
            MSFlowEntry cross_entry = findFlowIntersection_once(fe,one_entry);
            flowentries.push_back(cross_entry);
            if(!equalFlow(fe,cross_entry)){
                flowentries.push_back(findFlowDifference(fe,cross_entry));
            }
        }
    }else{
        std::vector<MSFlowEntry> tempflowentries;
        if(hasFlowIntersection(fe,one_entry)){
            MSFlowEntry cross_entry = findFlowIntersection_once(fe,one_entry);
            tempflowentries.push_back(cross_entry);
            if(!equalFlow(fe,cross_entry)){
                tempflowentries.push_back(findFlowDifference(fe,cross_entry));
            }
        }
        for(std::vector<MSFlowEntry>::iterator it=tempflowentries.begin();
                it!=tempflowentries.end();++it){
            std::vector<MSFlowEntry> temp = splitFlowEntry_with_subtask(*it,i+1,subtask);
            flowentries.insert(flowentries.end(),temp.begin(),temp.end());
        }
    }
    return flowentries;
}

std::vector<MSFlowEntry> splitFlowEntry_with_allsubtasks(MSFlowEntry fe, unsigned int i, 
                                                        SubTaskMap subtasks){ //将flowentry分割成多个flowentry
    std::vector<MSFlowEntry> flowentries;
    unsigned int map_size = subtasks.size();
    SubTaskMap::iterator it = subtasks.begin();
    if(i == map_size-1){ //最后一个任务
        std::map<unsigned int, MSSubTask>::iterator it2 = it->second.begin();
        std::vector<MSFlowEntry> temp = splitFlowEntry_with_subtask(fe,0,(it2+i)->second);
        return temp;
    }else{
        std::vector<MSFlowEntry> temp = splitFlowEntry_with_subtask(fe,0,it->second[i]);
        for(std::vector<MSFlowEntry>::iterator it=temp.begin();
                it!=temp.end();++it){
            std::vector<MSFlowEntry> temp2 = splitFlowEntry_with_allsubtasks(*it,i+1,subtasks);
            flowentries.insert(flowentries.end(),temp2.begin(),temp2.end());
        }
    }
    return flowentries;
}

float getSubTaskRatio(unsigned int subtask_id, std::map<unsigned int, float> &ratio){ //获取子任务的ratio
    unsigned int task_id = subtask_id>>22;
    return ratio[task_id];
}

std::map<MSFlowEntry,std::vector<unsigned int> >  getMeasurementObject(SubTaskMap _subtasks){ //获取测量对象碎片化集合
    //input
    std::map<MSFlowEntry,std::vector<unsigned int> > flow_subtask; //flow对应的subtask
    
    //分割flow
    for(SubTaskMap::iterator it = _subtasks.begin();
                it!=_subtasks.end();++it){ //遍历所有的subtask
        for(int i=0;i<it->second.size();++i){ //遍历每个subtask的flow
            for(int j=0;j<it->second[i].flow_num;++j){ //遍历每个flow
                std::vector<MSFlowEntry> flowentries = splitFlowEntry_with_subtask(it->second[i].flow[j],0,it->second[i]);
                for(std::vector<MSFlowEntry>::iterator it2 = flowentries.begin();
                        it2!=flowentries.end();++it2){ //遍历每个分割后的flow
                    if(flow_subtask.find(*it2)==flow_subtask.end()){ //如果map中没有这个flow
                        std::vector<unsigned int> temp;
                        temp.push_back(it->second[i].id.id2);
                        flow_subtask.insert(std::pair<MSFlowEntry,std::vector<unsigned int> >(*it2,temp));
                    }else{ //如果map中有这个flow
                        flow_subtask[*it2].push_back(it->second[i].id.id2);
                    }
                }
            }
        }
    }
    return flow_subtask;
}

std::map<unsigned int, float> getTaskRatio(std::map<MSFlowEntry,std::vector<unsigned int> > &flow_subtask){
    std::map<unsigned int, float> ratio;  //任务 - 流量负载比



    return ratio;
}

std::map<MSFlowEntry, float> getFlowTrafficRate(std::map<MSFlowEntry,std::vector<unsigned int> > &flow_subtask){
    std::map<MSFlowEntry, float> traffic_rate;  //FLowEntry - 流量速率



    return traffic_rate;
}


std::map<MSFlowEntry, float > getFlowTotalRatio( std::map<MSFlowEntry,std::vector<unsigned int> > &flow_subtask,
                                            std::map<unsigned int, float> ratio,
                                            std::map<MSFlowEntry, float> traffic_rate)
{
    std::map<MSFlowEntry, float > flow_subtask_ratio; //flow对应的subtask的总ratio
    //计算每个flow对应的ratio
    for(std::map<MSFlowEntry,std::vector<unsigned int> >::iterator it = flow_subtask.begin();
                it!=flow_subtask.end();++it){
        float r = 0;
        for(std::vector<unsigned int>::iterator it2 = it->second.begin();
                it2!=it->second.end();++it2){
            r += getSubTaskRatio(*it2,ratio)*traffic_rate[it->first];
        }
        flow_subtask_ratio.insert(std::pair<MSFlowEntry,float>(it->first,r));
    }
    return flow_subtask_ratio;
}

//建模求解
