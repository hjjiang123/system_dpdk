// PluginManager.cpp
#include "PluginManager.h"

PluginManager _PM;

PluginManager::PluginManager(){
}

PluginManager::~PluginManager(){
    for (auto& plugin : plugins_)
        dlclose(plugin.handle);
}

int PluginManager::loadPlugin(PluginInfo info){
    PluginHandle handle = NULL;
    handle = getPluginHandle(info.filename);
    if(handle == NULL){
        char filename[128];
        sprintf(filename, "%s/%s",PLUGIN_DIR,info.filename);
        handle = dlopen(filename, RTLD_LAZY);
    }
    if (handle == NULL)
        return -1;
    PluginHandleInfo *handleinfo = new PluginHandleInfo;
    handleinfo->info = info;
    handleinfo->handle = handle;
    plugins_.push_back(*handleinfo);
    printf("load plugin %s\n", info.filename);
    return 0;
}

bool PluginManager::unloadPlugin(const char filename[]){
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it){
        if (strcmp(it->info.filename,filename)==0){
            dlclose(it->handle);
            plugins_.erase(it);
            return true;
        }
    }
    return false;
}

bool PluginManager::unloadPlugin(unsigned int id){
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it){
        if (it->info.id.id2 == id){
            dlclose(it->handle);
            plugins_.erase(it);
            return true;
        }
    }
    return false;
}

PluginHandle PluginManager::getPluginHandle(const char filename[]){
    for (auto& plugin : plugins_){
        if (strcmp(plugin.info.filename,filename)==0)
            return plugin.handle;
    }
    return NULL;
}

PluginInfo *PluginManager::getPluginInfo_fromid(int id){
    for (auto& plugin : plugins_){
        if (plugin.info.id.id2 == id) {
            PluginInfo* info = new PluginInfo(plugin.info);
            return info;
        }
    }
    return nullptr;
}
