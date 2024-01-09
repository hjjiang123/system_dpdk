#ifndef SUBTASK_RESOLVE_H
#define SUBTASK_RESOLVE_H
#include <iostream>
#include <thread>
#include <unordered_map>
#include "../task.h"
#include "plugin.h"
#include <stdio.h> 
#include "SubTaskManager.h"
#include "FlowManager.h"
#include "../clienttest/client.h"
#include "dataplane.h"

void resolve_task_thread(MSSubTask *subtask);
void set_task_stopped_forced(unsigned int subtaskid);
bool get_task_running(unsigned int subtaskid);
#endif // SUBTASK_RESOLVE_H