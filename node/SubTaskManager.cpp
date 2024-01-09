// PluginManager.cpp
#include "SubTaskManager.h"

MSSubTaskHandleInfo _TM;

MSSubTaskManager::MSSubTaskManager(){
}

MSSubTaskManager::~MSSubTaskManager(){
    for (auto& task : tasks_)
        dlclose(task.handle);
}

int MSSubTaskManager::loadTask(MSSubTask *task){
    MSSubTaskHandle handle = NULL;
    handle = getSubTaskHandle(task->filename);
    char filename[128];
    sprintf(filename, "%s/%s",PLUGIN_DIR,task->filename);
    handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL)
        return -1;
    MSSubTaskHandleInfo *handleinfo = new MSSubTaskHandleInfo;
    handleinfo->task = task;
    handleinfo->handle = handle;
    tasks_.push_back(*handleinfo);
    printf("load plugin %s\n", task->filename);
    return 0;
}

bool MSSubTaskManager::unloadTask(const char filename[]){
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it){
        if (strcmp(it->task->filename,filename)==0){
            dlclose(it->handle);
            tasks_.erase(it);
            return true;
        }
    }
    return false;
}

bool MSSubTaskManager::unloadTask(unsigned int id){
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it){
        if (it->task->id.id2 == id){
            dlclose(it->handle);
            tasks_.erase(it);
            return true;
        }
    }
    return false;
}

MSSubTaskHandle MSSubTaskManager::getSubTaskHandle(const char filename[]){
    for (auto& task : tasks_){
        if (strcmp(task.task->filename,filename)==0)
            return task.handle;
    }
    return NULL;
}

MSSubTaskHandleInfo *MSSubTaskManager::getMSSubTaskHandleInfo_fromid(unsigned int id){
    for (auto& task : tasks_){
        if (task.task->id.id2 == id) {
            MSSubTaskHandleInfo* info = new MSSubTaskHandleInfo(task);
            return info;
        }
    }
    return nullptr;
}
