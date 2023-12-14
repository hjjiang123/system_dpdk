#ifndef TASK_RESOLVER_H
#define TASK_RESOLVER_H
#include "../task.h"
#include "../node/plugin.h"


//提供将任务中插件按照DAG进行解析并形成一个报文处理函数的功能


int resolveTask(MSTask task);



#endif