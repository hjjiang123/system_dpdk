#ifndef DATAPLANE_H
#define DATAPLANE_H
#include <mutex>
#include <unistd.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <rte_hash.h>
#include <rte_hash_crc.h>
#include <rte_mbuf.h>
#include "FlowManager.h"
#include "SubTaskManager.h"
#include "PluginRuntimeManager.h"
#include "QueueManager.h"
#include "command.h"
#include "capture.h"
#include "subtask_runtime.h"

struct lcoreCommandQueue {
    Command queue[SOCKET_QUEUE_SIZE];
    int front;
    int rear;
    std::mutex mt;
};

void registerSubTask(MSSubTask *task);

void unregisterSubTask(MSSubTask *task);

void unregisterSubTask(unsigned int subtask_id);

// void addMSSubTask(unsigned int subtask_id);

void deleteMSSubTask(unsigned int subtask_id);

int addFlowFilter(uint16_t port_id, uint32_t markid, uint32_t priority, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask);

void deleteFlowFilter(int id);

void addQueueToCore(int queueid, int coreid);

void deleteQueueFromCore(int queueid, int coreid);

bool push_Command(Command c);

SubTaskPerformance get_subtask_performance(unsigned int subtask_id);

void configurePort(unsigned int port_id);

void configureNumCores(unsigned int numcores);

void init(int argc, char **argv);

int handle_packet_per_core(void *arg);

void run();

#endif