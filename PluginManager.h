// PluginManager.h
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include <dlfcn.h>
#include "plugin.h"

//类内函数实现均在cpp
class PluginManager{
public:
    PluginManager();//构造函数
    ~PluginManager();//析构函数

    bool loadPlugin(const std::string& filename);//加载插件
    bool unloadPlugin(const std::string& filename);//卸载插件

    template<typename T> 
    T getFunction(const std::string& filename, const std::string& functionName="process"){
        PluginHandle handle = getPluginHandle(filename);
        if (handle == NULL)
            return NULL;

        void* proc = dlsym(handle, functionName.c_str());

        if (proc == NULL)
            return NULL;

        return reinterpret_cast<T>(proc);
    }
    typedef void* PluginHandle;
    PluginHandle getPluginHandle(const std::string& filename);

    PluginInfo *getPluginInfo_fromid(int id);
private:
    
    std::vector<PluginInfo> plugins_;

    
    int _nextId = 1; // 下一个可用的插件编号
};

#endif
