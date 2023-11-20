// PluginManager.cpp
#include "PluginManager.h"

PluginManager::PluginManager(){
}

PluginManager::~PluginManager(){
    for (auto& plugin : plugins_)
        dlclose(plugin.handle);
}

bool PluginManager::loadPlugin(const std::string& filename){
    PluginHandle handle = NULL;

    handle = dlopen(filename.c_str(), RTLD_LAZY);

    if (handle == NULL)
        return false;

    PluginInfo info;
    info.filename = filename;
    info.handle = handle;
    info.id = _nextId++;

    plugins_.push_back(std::move(info));

    return true;
}

bool PluginManager::unloadPlugin(const std::string& filename){
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it){
        if (it->filename == filename){
            dlclose(it->handle);
            plugins_.erase(it);
            return true;
        }
    }

    return false;
}

PluginManager::PluginHandle PluginManager::getPluginHandle(const std::string& filename){
    for (auto& plugin : plugins_){
        if (plugin.filename == filename)
            return plugin.handle;
    }

    PluginHandle handle = NULL;

    handle = dlopen(filename.c_str(), RTLD_LAZY);

    if (handle == NULL)
        return NULL;

    PluginInfo info;
    info.filename = filename;
    info.handle = handle;
    
    plugins_.push_back(info);

    return handle;
}

PluginInfo *PluginManager::getPluginInfo_fromid(int id){
    for (auto& plugin : plugins_){
        if (plugin.id == id)
            return &plugin;
    }
    return NULL;
}
