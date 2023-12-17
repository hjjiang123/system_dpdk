// PluginManager.h
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H
#include <stdio.h>
#include <vector>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include "plugin.h"

typedef void* PluginHandle;
struct PluginHandleInfo{
    PluginInfo info;
    PluginHandle handle;
};
class PluginManager;
extern PluginManager _PM;

//类内函数实现均在cpp
/**
 * @brief The PluginManager class manages the loading and unloading of plugins.
 */
class PluginManager{
public:
    /**
     * @brief Constructs a PluginManager object.
     */
    PluginManager();//构造函数

    /**
     * @brief Destructs the PluginManager object.
     */
    ~PluginManager();//析构函数

    /**
     * @brief Loads a plugin from the specified PluginInfo.
     * @param info The PluginInfo object containing the plugin information.
     * @return Returns 0 on success, or a negative value on failure.
     */
    int loadPlugin(PluginInfo info);//加载插件

    /**
     * @brief Unloads a plugin with the specified filename.
     * @param filename The filename of the plugin to unload.
     * @return Returns true if the plugin was successfully unloaded, false otherwise.
     */
    bool unloadPlugin(const char filename[]);//卸载插件
    /**
     * @brief Unloads a plugin with the specified filename.
     * @param filename The filename of the plugin to unload.
     * @return Returns true if the plugin was successfully unloaded, false otherwise.
     */
    bool unloadPlugin(unsigned int id);//卸载插件

    /**
     * @brief Retrieves a function pointer from a loaded plugin.
     * @tparam T The type of the function pointer.
     * @param filename The filename of the plugin.
     * @param functionName The name of the function to retrieve. Defaults to "process".
     * @return Returns the function pointer of type T if successful, or NULL if the function was not found.
     */
    template<typename T> 
    T getFunction(const char filename[], const char functionName[]){
        PluginHandle handle = getPluginHandle(filename);
        if (handle == NULL)
            return NULL;

        void* proc = dlsym(handle, functionName);

        if (proc == NULL)
            return NULL;

        return reinterpret_cast<T>(proc);
    }

    /**
     * @brief Retrieves the handle of a loaded plugin.
     * @param filename The filename of the plugin.
     * @return Returns the handle of the plugin if found, or NULL if the plugin was not loaded.
     */
    PluginHandle getPluginHandle(const char filename[]);

    /**
     * @brief Retrieves the PluginInfo object associated with the specified plugin ID.
     * @param id The ID of the plugin.
     * @return Returns a pointer to the PluginInfo object if found, or NULL if the plugin ID was not found.
     */
    PluginInfo *getPluginInfo_fromid(int id);
private:
    std::vector<PluginHandleInfo> plugins_; // Vector to store loaded plugins
    const char* PLUGIN_DIR = "./plugin";
};

#endif
