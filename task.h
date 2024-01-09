#ifndef TASK_H
#define TASK_H
#include "node/plugin.h"
#include <pthread.h>
// 交换机流规则
typedef struct MSFlowEntry{
    int direction; // 0->in, 1->out
    unsigned int src_ip;
    unsigned int src_mask;
    unsigned int dst_ip;
    unsigned int dst_mask;
    unsigned short src_port;
    unsigned short dst_port;
    unsigned char protocol;
} MSFlowEntry;

typedef struct MSFlowEntryPrioritized{
    MSFlowEntry flow;
    unsigned int priority;
} MSFlowEntryPrioritized;

//用户提交任务
typedef struct MSTask{
    int task_id; //任务号，由任务注册分配
    int flow_num; //流规则数量
    MSFlowEntry flow[16]; //流规则数组
    int obj_split; //目标可分性
    char filename[100];  /** MSTask Plugin filename */
    int pi_num; //插件数量
    PluginInfo pis[8]; //插件数组
    int pi_relations[8][8]; //插件DAG
    long long time; //任务持续秒数
    int epoch; //测量周期，秒数
    
} MSTask;

//子任务
typedef struct MSSubTask{
    int type; //0->add,1->delete,2->update
    union {
        struct 
        {
            unsigned int task_id:10; //任务号，由任务注册分配
            unsigned int subtask_id:10; //子任务号，由任务调度分配
            unsigned int reserve:12; //保留位
        } id1;
        unsigned int id2;
    } id;
    unsigned int host_id; //节点号，由任务调度分配
    unsigned int core_id;  // 核心号
    unsigned int flip;          // 是否翻转，0->否，1->是
    int flow_num; //流规则数量
    MSFlowEntryPrioritized flow[16]; //流规则数组
    char filename[100];  /** MSSubTask Plugin filename */
    int pi_num; //插件数量
    PluginInfo pis[8]; //插件数组
    int pi_relations[8][8]; //插件DAG
    long long times; //任务持续周期数
    int epoch; //测量周期，秒数

    pthread_mutex_t monitor_mt; // 互斥量
    int pos; //位置
    unsigned long long recore_tscs[60]; //记录点时钟周期数
    unsigned int recv_nums[60]; //接收包数量
    unsigned long long tsc_nums[60]; //该子任务运行时占用的时钟周期数

} MSSubTask;

//子任务运行时
typedef struct MSSubTaskRuntime{
    MSSubTask *subtask;
    int pi_num; //插件数量
    int pi_relations[8][8]; //插件DAG
    PluginRuntime pis[8]; //插件运行时数组

    unsigned long long recore_tsc; //记录点时钟周期数
    unsigned int recv_num; //接收包数量
    unsigned long long tsc_num; //该子任务运行时占用的时钟周期数
} MSSubTaskRuntime;

typedef struct SubTaskPerformance{
    unsigned int task_id; //任务号
    unsigned int inner_subtask_id; //子任务号
    int pos; //位置
    unsigned long long recore_tscs[60]; //记录点时钟周期数
    unsigned int recv_nums[60]; //接收包数量
    unsigned long long tsc_nums[60]; //该子任务运行时占用的时钟周期数
} SubTaskPerformance;

#endif