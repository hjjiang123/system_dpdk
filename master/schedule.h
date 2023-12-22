#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <map>
#include <mutex>
#include <queue>
#include <iostream>
#include <vector>
#include "../task.h"
#include "../node/plugin.h"
typedef struct FlowEntryDupInfo{
    int task_id;
    int subtask_id;
    int flow_index;
    MSFlowEntry flow[16];
} FlowEntryDupInfo;

MSFlowEntry findFlowDifference(MSFlowEntry fe1, MSFlowEntry fe2); // fe1-fe2
bool equalFlow(MSFlowEntry fe1, MSFlowEntry fe2); // fe1==fe2
MSFlowEntry findFlowIntersection_once(MSFlowEntry fe1, MSFlowEntry fe2);// fe1&fe2
bool hasFlowIntersection(MSFlowEntry fe1, MSFlowEntry fe2);// fe1&fe2!=0

//维护待加载的任务

//维护已加载的任务

//维护节点已调度流规则

//维护交换机已调度流规则

//维护已调度的子任务

//调度任务生成子任务
void generateSubTask(MSTask& task);
//调度子任务到节点

#endif