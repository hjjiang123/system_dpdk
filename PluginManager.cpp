// PluginManager.cpp
#include "PluginManager.h"

PluginManager::PluginManager(){
}

PluginManager::~PluginManager(){
    for (auto& plugin : plugins_)
        dlclose(plugin.handle);
}

int PluginManager::loadPlugin(PluginInfo info){
    PluginHandle handle = NULL;

    handle = dlopen(info.filename, RTLD_LAZY);

    if (handle == NULL)
        return -1;
    int id = _nextId++;
    info.id = id;
    PluginHandleInfo handleinfo={info, handle};
    plugins_.push_back(std::move(handleinfo));
    return id;
}

bool PluginManager::unloadPlugin(const char filename[]){
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it){
        if (it->info.filename == filename){
            dlclose(it->handle);
            plugins_.erase(it);
            return true;
        }
    }

    return false;
}

PluginHandle PluginManager::getPluginHandle(const char filename[]){
    for (auto& plugin : plugins_){
        if (plugin.info.filename == filename)
            return plugin.handle;
    }
    return NULL;
}

PluginInfo *PluginManager::getPluginInfo_fromid(int id){
    for (auto& plugin : plugins_){
        if (plugin.info.id == id) {
            PluginInfo* info = new PluginInfo(plugin.info);
            return info;
        }
    }
    return nullptr;
}
