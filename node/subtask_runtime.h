#ifndef SUBTASK_RUNTIME_H
#define SUBTASK_RUNTIME_H
#include <sys/stat.h>
#include "plugin.h"
#include "PluginManager.h"
#include "config.h"
#include "../task.h"
typedef struct MSSubTaskRuntimeNode
{
    MSSubTaskRuntime trt;
    struct MSSubTaskRuntimeNode *next;
} MSSubTaskRuntimeNode;

typedef struct MSSubTaskRuntimeNodeList
{
    MSSubTaskRuntimeNode *head;
    MSSubTaskRuntimeNode *tail;
} MSSubTaskRuntimeNodeList;

extern MSSubTaskRuntimeNodeList _handlers_[MAX_CORE_NUMS]; 

void initMSSubTaskRuntimeNodeList();
bool getMSSubTaskRuntimeNode(unsigned int subtask_id, MSSubTaskRuntimeNode *node);
MSSubTaskRuntimeNode * allocateMSSubTaskRuntime(unsigned int subtask_id);
void addMSSubTaskRuntime(MSSubTaskRuntimeNode *trtnode);
bool popMSSubTaskRuntime(unsigned int subtask_id, MSSubTaskRuntimeNode *node);
void deleteMSSubTaskRuntime(unsigned int subtask_id);
void deleteMSSubTaskRuntimePoped(MSSubTaskRuntimeNode *node);
void dumpMSSubTaskRuntimeNode(MSSubTaskRuntimeNode *node);
bool enqueueMSSubTaskRuntimeNodeDump(MSSubTaskRuntimeNode *node);
void dequeueMSSubTaskRuntimeNodeDump();

#endif