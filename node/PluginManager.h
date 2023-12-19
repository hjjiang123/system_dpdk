#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H
#include <stdio.h>
#include <vector>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include "plugin.h"
#include "task.h"

typedef void* MSSubTaskHandle;
struct MSSubTaskHandleInfo{
    MSSubTask task;
    MSSubTaskHandle handle;
};
class MSSubTaskManager;
extern MSSubTaskManager _TM;

class MSSubTaskManager{
public:
    MSSubTaskManager();
    ~MSSubTaskManager();
    int loadTask(MSSubTask task);
    bool unloadTask(const char filename[]);
    bool unloadTask(unsigned int id);
    template<typename T> 
    T getFunction(const char filename[], const char functionName[]){
        MSSubTaskHandle handle = getSubTaskHandle(filename);
        if (handle == NULL)
            return NULL;
        void* proc = dlsym(handle, functionName);
        if (proc == NULL)
            return NULL;
        return reinterpret_cast<T>(proc);
    }
    MSSubTaskHandle getSubTaskHandle(const char filename[]);
    MSSubTaskHandleInfo *getMSSubTaskHandleInfo_fromid(unsigned int id);
private:
    std::vector<MSSubTaskHandleInfo> tasks_;
    const char* PLUGIN_DIR = "./plugin";
};

#endif
