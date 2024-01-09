#include "subtask_runtime.h"

#include <iostream>
#include <vector>

#define MAX_PLUGIN_NUMBER_IN_CORE 10

bool registrationNumbers[MAX_CORE_NUMS][MAX_PLUGIN_NUMBER_IN_CORE]; // 用于表示注册号的分配情况

unsigned int get_plugin_index(int core_id) {
    for (int i = 0; i < MAX_PLUGIN_NUMBER_IN_CORE; i++) {
        if (!registrationNumbers[core_id][i]) {
            registrationNumbers[core_id][i] = true; // 标记为已分配
            return 1<<i;
        }
    }
    return -1; // 没有可用的未分配注册号
}

void release_plugin_index(int core_id, unsigned int number) {
    int index = 0;
    while(number % 2 == 0) {
        number = number >> 1;
        index ++;
    }
    if (number >= 0 && number < MAX_PLUGIN_NUMBER_IN_CORE) {
        registrationNumbers[core_id][index] = false; // 标记为未分配
    }
}

MSSubTaskRuntimeNodeList _handlers_[MAX_CORE_NUMS];

MSSubTaskRuntimeNode *taskdumpnodequeue[MAX_DUMP_NUMS]; // Plugin runtime nodes for dumping results
int mstask_nodequeue_front = 0;  // Front index of the queue
int mstask_nodequeue_rear = 0;   // Rear index of the queue
int mstask_nodequeue_count = 0;  // Number of elements in the queue

pthread_mutex_t mstask_nodequeue_mutex = PTHREAD_MUTEX_INITIALIZER;    // Mutex lock
pthread_cond_t mstask_nodequeue_cond_empty = PTHREAD_COND_INITIALIZER; // Condition variable for empty queue


void initMSSubTaskRuntimeNodeList()
{
    for (int i = 0; i < MAX_CORE_NUMS; i++)
    {
        _handlers_[i].head = NULL;
        _handlers_[i].tail = NULL;
    }
}

MSSubTaskRuntimeNode * allocateMSSubTaskRuntime(unsigned int subtask_id)
{
    MSSubTaskRuntimeNode *trtnode=(MSSubTaskRuntimeNode *)malloc(sizeof(MSSubTaskRuntimeNode));
    MSSubTaskRuntime trt;
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    trt.subtask = pi->task;
    trt.pi_num=pi->task->pi_num;
    for(int i=0;i<pi->task->pi_num;i++){
        for(int j=0;j<pi->task->pi_num;j++){
            trt.pi_relations[i][j]=pi->task->pi_relations[i][j];
        }
    }
    // Allocate resources for the plugins
    for(int n=0;n<pi->task->pi_num;n++){
        // Allocate memory for the result
        Byte ****res = (Byte ****)malloc(pi->task->pis[n].cnt_info.rownum * sizeof(Byte ***));
        for (int i = 0; i < pi->task->pis[n].cnt_info.rownum; i++)
        {
            res[i] = (Byte ***)malloc(pi->task->pis[n].cnt_info.bucketnum * sizeof(Byte **));
            for (int j = 0; j < pi->task->pis[n].cnt_info.bucketnum; j++)
            {
                res[i][j] = (Byte **)malloc(pi->task->pis[n].cnt_info.bucketsize * sizeof(Byte *));
                for (int k = 0; k < pi->task->pis[n].cnt_info.bucketsize; k++)
                {
                    res[i][j][k] = (Byte *)malloc(pi->task->pis[n].cnt_info.countersize * sizeof(Byte));
                }
            }
        }
        // Get the function pointer for the plugin
        PF myFunctionPtr = (PF)_TM.getFunction<PF>(pi->task->pis[n].filename, pi->task->pis[n].funcname);
        // Create a plugin runtime object
        
        trt.pis[n].res = res;
        trt.pis[n].id.id1.core_id = pi->task->core_id;
        unsigned int subtask_index = get_plugin_index(pi->task->core_id);
        trt.pis[n].id.id1.subtask_index = subtask_index;
        trt.pis[n].id.id1.flip = pi->task->flip == 1 ? subtask_index:0;
        trt.pis[n].id.id1.reserve = 0;
        trt.pis[n].func = myFunctionPtr;
    }
    trtnode->trt=trt;
    trtnode->next=NULL;
    return trtnode;
}
void addMSSubTaskRuntime(MSSubTaskRuntimeNode *trtnode)
{
    int coreid = trtnode->trt.subtask->core_id;
    if (_handlers_[coreid].head == NULL)
    {
        _handlers_[coreid].head = trtnode;
        _handlers_[coreid].tail = trtnode;
    }
    else
    {
        _handlers_[coreid].tail->next = trtnode;
        _handlers_[coreid].tail = trtnode;
    }
}
bool popMSSubTaskRuntime(unsigned int subtask_id, MSSubTaskRuntimeNode *node)
{
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    int coreid = pi->task->core_id;
    node = _handlers_[coreid].head;
    MSSubTaskRuntimeNode *previous = NULL;
    while (node != NULL)
    {
        if (node->trt.subtask->id.id2 == subtask_id)
        {
            if (previous == NULL)
            {
                _handlers_[coreid].head = node->next;
            }
            else
            {
                previous->next = node->next;
            }

            if (node == _handlers_[coreid].tail)
            {
                _handlers_[coreid].tail = previous;
            }
            return true;
        }
        previous = node;
        node = node->next;
    }
    return false;
}

bool getMSSubTaskRuntimeNode(unsigned int subtask_id, MSSubTaskRuntimeNode *node)
{
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    int coreid = pi->task->core_id;
    MSSubTaskRuntimeNode * nodetemp = _handlers_[coreid].head;
    while (nodetemp != NULL)
    {
        if (node->trt.subtask->id.id2 == subtask_id)
        {
            node = nodetemp;
            return true;
        }
        nodetemp = nodetemp->next;
    }
    return false;
}

void deleteMSSubTaskRuntimePoped(MSSubTaskRuntimeNode *node){
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(node->trt.subtask->id.id2);
    for(int n=0;n<pi->task->pi_num;n++){
        // Release memory
        for (int i = 0; i < pi->task->pis[n].cnt_info.rownum; i++)
        {
            for (int j = 0; j < pi->task->pis[n].cnt_info.bucketnum; j++)
            {
                for (int k = 0; k < pi->task->pis[n].cnt_info.bucketsize; k++)
                {
                    free((void *)(node->trt.pis[n].res)[i][j][k]);
                    node->trt.pis[n].res[i][j][k] = NULL;
                }
                free((void *)(node->trt.pis[n].res)[i][j]);
                node->trt.pis[n].res[i][j] = NULL;
            }
            free((void *)(node->trt.pis[n].res[i]));
            node->trt.pis[n].res[i] = NULL;
        }
        free((void *)node->trt.pis[n].res);
        node->trt.pis[n].res = NULL;
    }
    release_plugin_index(pi->task->core_id, node->trt.pis[0].id.id1.subtask_index);
    free(node);
    node = NULL;
}
void deleteMSSubTaskRuntime(unsigned int subtask_id)
{
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    int coreid = pi->task->core_id;
    MSSubTaskRuntimeNode *node = NULL;
    if(popMSSubTaskRuntime(subtask_id, node)){
        for(int n=0;n<pi->task->pi_num;n++){
            // Release memory
            for (int i = 0; i < pi->task->pis[n].cnt_info.rownum; i++)
            {
                for (int j = 0; j < pi->task->pis[n].cnt_info.bucketnum; j++)
                {
                    for (int k = 0; k < pi->task->pis[n].cnt_info.bucketsize; k++)
                    {
                        free((void *)(node->trt.pis[n].res)[i][j][k]);
                        node->trt.pis[n].res[i][j][k] = NULL;
                    }
                    free((void *)(node->trt.pis[n].res)[i][j]);
                    node->trt.pis[n].res[i][j] = NULL;
                }
                free((void *)(node->trt.pis[n].res[i]));
                node->trt.pis[n].res[i] = NULL;
            }
            free((void *)node->trt.pis[n].res);
            node->trt.pis[n].res = NULL;
        }
        release_plugin_index(pi->task->core_id, node->trt.pis[0].id.id1.subtask_index);
        free(node);
        node = NULL;
    }
}
void createSubdirectory(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}
// Check if subdirectory exists, create if not
void checkAndCreateSubdirectory(MSSubTaskRuntimeNode* node) {
    char subdirectoryPath[128];
    sprintf(subdirectoryPath, "./res/%d", node->trt.subtask->id);
    createSubdirectory(subdirectoryPath);
}
void dumpMSSubTaskRuntimeNode(MSSubTaskRuntimeNode *node){
    printf("dumpResult\n");
    checkAndCreateSubdirectory(node);
    for(int n=0;n<node->trt.pi_num;n++){
        char p[128];
        sprintf(p,"./res/%d/%d",node->trt.subtask->id,n);
        FILE *file = fopen(p, "wb");
        if (file == NULL) {
            for (int i = 0; i < node->trt.subtask->pis[n].cnt_info.rownum; i++)
            {
                for (int j = 0; j < node->trt.subtask->pis[n].cnt_info.bucketnum; j++)
                {
                    for (int k = 0; k < node->trt.subtask->pis[n].cnt_info.bucketsize; k++)
                    {
                        fwrite(node->trt.pis[n].res[i][j][k], node->trt.subtask->pis[n].cnt_info.countersize, 1, file);
                        memset(node->trt.pis[n].res[i][j][k], 0, node->trt.subtask->pis[n].cnt_info.countersize);
                    }
                }
            }
        }
        fclose(file);
    }
}




// Enqueue a plugin runtime node
bool enqueueMSSubTaskRuntimeNodeDump(MSSubTaskRuntimeNode *nodedump) {
    pthread_mutex_lock(&mstask_nodequeue_mutex);
    if (mstask_nodequeue_count == MAX_DUMP_NUMS) {
        return false;
    }
    taskdumpnodequeue[mstask_nodequeue_rear] = nodedump;
    mstask_nodequeue_rear = (mstask_nodequeue_rear + 1) % MAX_DUMP_NUMS;
    mstask_nodequeue_count++;

    pthread_cond_signal(&mstask_nodequeue_cond_empty);
    pthread_mutex_unlock(&mstask_nodequeue_mutex);
    return true;
}

// Dequeue a plugin runtime node
void dequeueMSSubTaskRuntimeNodeDump() {
    pthread_mutex_lock(&mstask_nodequeue_mutex);
    while (mstask_nodequeue_count == 0) {
        pthread_cond_wait(&mstask_nodequeue_cond_empty, &mstask_nodequeue_mutex);
    }
    MSSubTaskRuntimeNode * item = taskdumpnodequeue[mstask_nodequeue_front];
    mstask_nodequeue_front = (mstask_nodequeue_front + 1) % MAX_DUMP_NUMS;
    mstask_nodequeue_count--;
    pthread_mutex_unlock(&mstask_nodequeue_mutex);
    dumpMSSubTaskRuntimeNode(item);
}
