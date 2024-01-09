#include "schedule.h"


 
const unsigned int _resource[10][40]=
                {100,100,100,100,100,100,100,100,100,100,};
const unsigned int _host_num = 10;
const unsigned int _core_num = 40*10;

typedef std::map<unsigned int,MSTask> TaskMap; //任务号 - 任务集合 
TaskMap tasks;  //维护当前已加载的任务

typedef std::map<unsigned int, std::map<unsigned int,MSSubTask> > SubTaskMap; //任务号 - 子任务集合
SubTaskMap subtasks; //维护当前已调度的子任务

std::map<MSFlowEntryPrioritized,std::vector<unsigned int> > flow_task; //维护每个流规则对应的任务集合

std::queue<MSTask> taskQueue; //维护待加载的任务
std::mutex queueMutex; //维护待加载的任务的互斥锁

//注册MSTask并分配task_id
unsigned int registerMSTask(MSTask& task) {
    //分配一个不重复的task_id
    int task_id = 0;
    while (tasks.find(task_id) != tasks.end()) {
        task_id++;
    }
    //为MSTask赋值task_id属性
    task.task_id = task_id;
    tasks[task_id] = task;
    subtasks[task_id] = std::map<unsigned int,MSSubTask>();
    return task_id;
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
    if(e1.direction = -1) return e2;
    MSFlowEntry e;
    return e;
};
bool hasFlowIntersection(MSFlowEntry e1, MSFlowEntry e2){
    if(e1.direction = -1) return true;
    return false;
}
MSFlowEntry findFlowDifference(MSFlowEntry e1, MSFlowEntry e2){ //e1-e2
    MSFlowEntry diff;
    // Perform set difference operation on the flow entries
    // and store the result in 'diff'
    // ...
    return diff;
};
/********************************调度功能函数****************************************/
std::vector<MSFlowEntryPrioritized> prioritize_flowentry_with_onetask( 
    MSFlowEntryPrioritized fe,
    MSTask onetask)
{
    std::vector<MSFlowEntryPrioritized> temp_flowentries;
    temp_flowentries.push_back(fe);
    for(int i=0;i < onetask.flow_num;i++){ //遍历每个flow
        if(hasFlowIntersection(fe.flow,onetask.flow[i])){
            MSFlowEntryPrioritized cross_entry_prioritized;
            cross_entry_prioritized.flow = findFlowIntersection_once(fe.flow,onetask.flow[i]);
            cross_entry_prioritized.priority = fe.priority+1;
            temp_flowentries.push_back(cross_entry_prioritized);
        }
    }
    return temp_flowentries;
}

std::vector<MSFlowEntryPrioritized> prioritize_flowentry_with_alltasks( 
    MSFlowEntryPrioritized fe,
    int i,
    TaskMap &tasks)
{
    std::vector<MSFlowEntryPrioritized> temp_flowentries;
    unsigned int map_size = tasks.size();
    std::map<unsigned int,MSTask>::iterator it = tasks.begin();
    if(i == map_size-1){ //最后一个任务
        std::advance(it, i);
        std::vector<MSFlowEntryPrioritized> temp = prioritize_flowentry_with_onetask(fe,it->second);
        return temp;
    }else{
        std::advance(it, i);
        std::vector<MSFlowEntryPrioritized> temp = prioritize_flowentry_with_onetask(fe,it->second);
        for(std::vector<MSFlowEntryPrioritized>::iterator it=temp.begin();
                it!=temp.end();++it){
            std::vector<MSFlowEntryPrioritized> temp2 = prioritize_flowentry_with_alltasks(*it,i+1,tasks);
            temp_flowentries.insert(temp_flowentries.end(),temp2.begin(),temp2.end());
        }
    }
    return temp_flowentries;
}


int findMSFlowEntryPriority(MSFlowEntry fe, std::vector<MSFlowEntryPrioritized>& flowentry_prioritized){
    for(int i=0;i<flowentry_prioritized.size();++i){
        if(equalFlow(fe,flowentry_prioritized[i].flow)){
            return i;
        }
    }
    return -1;
}

// 查找一个任务中每个流规则包含的交集流规则
std::vector<std::vector<MSFlowEntryPrioritized>> findAllCrossMSFlowEntryPriority(MSTask task, std::vector<MSFlowEntryPrioritized>& flowentry_prioritized){
    std::vector<std::vector<MSFlowEntryPrioritized>> crossset;
    for(int i=0;i<task.flow_num;i++){
        std::vector<MSFlowEntryPrioritized> temp;
        for(int j=0;j<flowentry_prioritized.size();j++){
            if(contains(task.flow[i],flowentry_prioritized[j].flow)){
                temp.push_back(flowentry_prioritized[j]);
            }
        }
        std::sort(temp.begin(),temp.end(),[](MSFlowEntryPrioritized a, MSFlowEntryPrioritized b){return a.priority < b.priority;});
        crossset.push_back(temp);
    }
    return crossset;
}

//查找一个任务中的一个流规则对应交集流规则集合中是否存在给定优先级的流规则
bool haveCrossMSFlowEntryByPriority(std::vector<MSFlowEntryPrioritized>& ordered_crossset, int priority){
    for(int i=0;i<ordered_crossset.size();i++){
        if(ordered_crossset[i].priority == priority){
            return true;
        }
    }
    return false;
}

//找到一个任务中的一个流规则对应交集流规则集合中所有给定优先级的流规则
std::vector<MSFlowEntryPrioritized> findCrossMSFlowEntryByPriority(std::vector<MSFlowEntryPrioritized>& ordered_crossset, int priority){
    std::vector<MSFlowEntryPrioritized> temp;
    for(int i=0;i<ordered_crossset.size();i++){
        if(ordered_crossset[i].priority == priority){
            temp.push_back(ordered_crossset[i]);
        }
    }
    return temp;
}

//根据交集流规则集合，生成一个流规则分割后流规则集合
std::vector<std::vector<MSFlowEntryPrioritized>> splitedMSFlowEntry(std::vector<MSFlowEntryPrioritized>& ordered_crossset){
    std::vector<std::vector<MSFlowEntryPrioritized>> flow_splited; // cross --> crosses to eliminate
    int lower = ordered_crossset[0].priority;
    int higher = ordered_crossset[ordered_crossset.size()-1].priority;
    int greater = lower+1;
    std::vector<MSFlowEntryPrioritized> temp_lower = findCrossMSFlowEntryByPriority(ordered_crossset,lower);
    while(greater<=higher){
        while(!haveCrossMSFlowEntryByPriority(ordered_crossset,greater)){
            greater++;
        }
        std::vector<MSFlowEntryPrioritized> temp_higher = findCrossMSFlowEntryByPriority(ordered_crossset,greater);
        for(int i=0;i<temp_lower.size();i++){
            std::vector<MSFlowEntryPrioritized> temp = temp_higher;
            temp.insert(temp.begin(),temp_lower[i]);
            flow_splited.push_back(temp);
        }
        temp_lower = temp_higher;
        greater = greater+1;
    }
    for(int i=0;i<temp_lower.size();i++){
        std::vector<MSFlowEntryPrioritized> temp;
        temp.push_back(temp_lower[i]);
        flow_splited.push_back(temp);
    }
    return flow_splited;
}


// 获取测量对象碎片化集合
void getMeasurementObject(TaskMap &tasks, std::map<MSFlowEntryPrioritized,std::vector<MSFlowEntryPrioritized> > &flowdifference_map){ //获取测量对象碎片化集合
    //1.求交集
    MSFlowEntryPrioritized fe;
    fe.flow.direction = -1;
    fe.priority = 0;
    std::vector<MSFlowEntryPrioritized> flowentries = prioritize_flowentry_with_alltasks(fe,0,tasks);
    flowentries.erase(flowentries.begin());

    //2.去重
    std::set<int> to_delete;
    for(int i=0;i<flowentries.size();i++){
        for(int j=0;j<flowentries.size();j++){
            if(j!=i && contains(flowentries[i].flow, flowentries[j].flow ) && flowentries[i].priority >= flowentries[j].priority){
                to_delete.insert(j);
            }
        }
    }
    std::vector<int> sorted_to_delete(to_delete.begin(), to_delete.end());
    std::sort(sorted_to_delete.begin(), sorted_to_delete.end());
    for(int i=sorted_to_delete.size()-1;i>=0;i--){
        flowentries.erase(flowentries.begin()+sorted_to_delete[i]);
    }

    //3.获取每个任务的flowentry的差集表示
    std::map<unsigned int,std::vector<std::vector<std::vector<MSFlowEntryPrioritized>>> > task_flowentry_splited; //任务 - flowentry列表
    for(TaskMap::iterator it = tasks.begin();
                it!=tasks.end();++it){ //遍历所有的task
        std::vector<std::vector<MSFlowEntryPrioritized>> onetask_cross 
                    = findAllCrossMSFlowEntryPriority(it->second,flowentries);//获取一个任务中每个流规则包含的交集流规则
        for(int i=0;i<onetask_cross.size();i++){
            std::vector<std::vector<MSFlowEntryPrioritized>> oneflowentry_splited 
                        = splitedMSFlowEntry(onetask_cross[i]);//获取一个流规则分割后流规则集合
            task_flowentry_splited[it->first].push_back(oneflowentry_splited);
        }
    }
    //4.获取所有分割后flowentry的差集集合，即测量目标单元的集合;以及对应的任务列表
    for(std::map<unsigned int,std::vector<std::vector<std::vector<MSFlowEntryPrioritized>>> >::iterator it = task_flowentry_splited.begin();
                it!=task_flowentry_splited.end();++it){ //遍历所有的task
        for(int i=0;i<it->second.size();i++){
            std::vector<std::vector<MSFlowEntryPrioritized>> oneflowentry_splited = it->second[i];
            for(int j=0;j<oneflowentry_splited.size();j++){
                std::vector<MSFlowEntryPrioritized> oneflowentry_difference = oneflowentry_splited[j];
                if(flowdifference_map.find(oneflowentry_difference[0]) == flowdifference_map.end()){ //如果map中没有这个oneflowentry_difference
                    std::vector<unsigned int> temp;
                    temp.push_back(it->first);
                    flow_task.insert(std::make_pair(oneflowentry_difference[0],temp));
                    flowdifference_map.insert(std::make_pair(oneflowentry_difference[0],oneflowentry_difference));
                }else{ //如果flow_task中有这个flow
                    flow_task[oneflowentry_difference[0]].push_back(it->first);
                }
            }
        }
    }
}

//获取每个任务的流量负载比
std::map<unsigned int, float> measureTaskRatio(
    std::vector<std::vector<SubTaskPerformance>> &stpfs){
    
    std::map<unsigned int, float> ratios;  //任务 - 流量负载比
    for(int i=0;i<stpfs.size();i++){
        for(int j=0; j<stpfs[i].size(); j++){
            unsigned int taskid = stpfs[i][j].task_id;
            unsigned long long r = stpfs[i][j].tsc_nums[stpfs[i][j].pos] / stpfs[i][j].recore_tscs[stpfs[i][j].pos];
            if (ratios.find(taskid) != ratios.end()) {
                ratios[taskid] += r;
            } else {
                ratios[taskid] = r;
            }
        }
    }
    // Iterate over ratios
    for (const auto& pair : ratios) {
        unsigned int taskid = pair.first;
        ratios[taskid] /= subtasks[taskid].size();
    }

    return ratios;
}

// 单节点上每个MSFlowEntryPrioritized的流量速率
std::map<MSFlowEntryPrioritized, float> measureFlowTrafficRateOneHost(
    std::vector<SubTaskPerformance> &stpf){

    std::map<MSFlowEntryPrioritized, float> traffic_rate;  //FLowEntry - 流量速率
    //计算每个MSFlowEntryPrioritized的流量速率
    for(int i=0; i<stpf.size(); i++){
        unsigned int taskid = stpf[i].task_id;
        unsigned int subtaskid = stpf[i].inner_subtask_id;
        MSFlowEntryPrioritized msfep = subtasks[taskid][subtaskid].flow[0];
        traffic_rate[msfep] = stpf[i].recv_nums[stpf[i].pos] / stpf[i].recore_tscs[stpf[i].pos];
    }
    return traffic_rate;
}

// 所有节点上每个MSFlowEntryPrioritized的流量速率
std::map<MSFlowEntryPrioritized, float> measureFlowTrafficRateAllHosts(
    std::vector<std::vector<SubTaskPerformance>> &stpfs){

    std::map<MSFlowEntryPrioritized, float> traffic_rate;
    for(int i=0; i<stpfs.size(); i++){
        std::map<MSFlowEntryPrioritized, float> rate = measureFlowTrafficRateOneHost(stpfs[i]);
        for (const auto& pair : rate) {
            const MSFlowEntryPrioritized& key = pair.first;
            float value = pair.second;
            if (traffic_rate.find(key) != traffic_rate.end()) {
                traffic_rate[key] += value;
            } else {
                traffic_rate[key] = value;
            }
        }
    }
    return traffic_rate;
}


// 获取每个MSFlowEntryPrioritized的CPU资源消耗=流量速率*流量负载比
std::map<MSFlowEntryPrioritized, float> 
getFlowTotalRatio(  std::map<unsigned int, float> &ratios,
                    std::map<MSFlowEntryPrioritized, float> &traffic_rate)
{
    std::map<MSFlowEntryPrioritized, float > flow_task_ratio; //flow对应的subtask的总ratio
    //计算每个flow对应的ratio
    for(std::map<MSFlowEntryPrioritized,std::vector<unsigned int> >::iterator it = flow_task.begin();
                it!=flow_task.end();++it){
        float r = 0;
        for(std::vector<unsigned int>::iterator it2 = it->second.begin();
                it2!=it->second.end();++it2){
            r += ratios[*it2];
        }
        r *= traffic_rate[it->first];
        flow_task_ratio.insert(std::pair<MSFlowEntryPrioritized,float>(it->first,r));
    }
    return flow_task_ratio;
}

// 获取每个节点上每个核心的CPU资源消耗
std::map<unsigned int, std::map<unsigned int, float> > 
calcu_cpu_used_rate(  std::map<unsigned int, float> &ratios,
                std::map<MSFlowEntryPrioritized, float> &traffic_rate){
    
    std::map<unsigned int, std::map<unsigned int, float> > cpu_used_rate; //节点 - 核心 - CPU资源消耗
    
    for (const auto& subtask : subtasks) {
        unsigned int taskid = subtask.first;
        for (const auto& pair : subtask.second) {
            unsigned int subtaskid = pair.first;
            unsigned int hostid = pair.second.host_id;
            unsigned int coreid = pair.second.core_id;
            float r = ratios[taskid] * traffic_rate[pair.second.flow[0]];
            if (cpu_used_rate.find(hostid) != cpu_used_rate.end()) {
                if (cpu_used_rate[hostid].find(coreid) != cpu_used_rate[hostid].end()) {
                    cpu_used_rate[hostid][coreid] += r;
                } else {
                    cpu_used_rate[hostid][coreid] = r;
                }
            } else {
                cpu_used_rate[hostid][coreid] = r;
            }
        }    
    }   
    return cpu_used_rate;
}

// 获取每个节点上每个节点的内存资源消耗
std::map<unsigned int, unsigned long long> mem_used(){
    std::map<unsigned int, unsigned long long> mem_used; //节点 - 内存资源消耗
    for (const auto& subtask : subtasks) {
        unsigned int taskid = subtask.first;
        for (const auto& pair : subtask.second) {
            unsigned int subtaskid = pair.first;
            unsigned int hostid = pair.second.host_id;
            unsigned long long mm = 0;
            for(int i=0;i<pair.second.pi_num;i++){
                CounterInfo ci = pair.second.pis[i].cnt_info;
                mm += ci.rownum*ci.bucketnum*ci.bucketsize*ci.countersize;
            }
            if (mem_used.find(hostid) != mem_used.end()) {
                mem_used[hostid] += mm;
            } else {
                mem_used[hostid] = mm;
            }
        }    
    }
    return mem_used;
}

/*
调度新到来的一个任务
*/
std::map<unsigned int,std::vector<MSSubTask>> schedule_new_task(MSTask& mst){
    // 1.注册任务，获取任务号
    unsigned int taskid = registerMSTask(mst);
    // 2.获取集群状态
    // 2.1 获取任务运行性能数据
    std::vector<std::vector<SubTaskPerformance>> stpfs = monitor();
    // 2.2 获取集群资源消耗数据
    std::map<unsigned int, float> ratios = measureTaskRatio(stpfs); //任务 - 流量负载比
    std::map<MSFlowEntryPrioritized, float> traffic_rate = measureFlowTrafficRateAllHosts(stpfs); //FLowEntry - 流量速率
    std::map<unsigned int, std::map<unsigned int, float> > used_cpu_rate = calcu_cpu_used_rate(ratios,traffic_rate);//节点 - 核心 - CPU资源消耗
    std::map<unsigned int, unsigned long long> used_mem = mem_used();           //节点 - 内存资源消耗
    // 3.判断任务是否可以分割？
    if(mst.obj_split==0){ //不可分割
        // 查找在当前集群中，该任务是否可以分配？
        for (const auto& subtask : subtasks) {
            for(const auto& pair : subtask.second){
                if(contains(pair.second.flow[0].flow, MSFlowEntry fe2))
            }
        }
    }
    
}


/*
重调度所有任务
*/

//建模求解
