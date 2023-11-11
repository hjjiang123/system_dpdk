#ifndef DATAPLANE_H
#define DATAPLANE_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>
#include "capture.h"
#include "plugin.h"


// plugin_id,plugin数组
// 注册plugin
//

class DataPlane
{
    unsigned int _port_id = 0; // 需要抓取网卡的索引号
    std::map<unsigned int, std::vector<std::unique_ptr<PluginInterface>>> _handlers;  //运行在各个核的插件列表
    std::map<unsigned int, std::vector<std::unique_ptr<PluginInterface>>> _handlers_updated; //待更新的各个核的插件列表
    std::map<int, std::unique_ptr<PluginInterface>> _plugins;  //插件库

    static int _nextId; // 下一个可用的插件编号

    std::vector<std::mutex> _mt;
    std::vector<int> _record;
    unsigned int _num_cores; // 获取可用的核心数量

public:
    DataPlane();
    void configurePort(unsigned int _port_id);
    void configureNumCores(unsigned int numcores);
    void init(int argc, char **argv);
    void addPlugin(std::unique_ptr<PluginInterface> newPlugin);
    void handle_packet_per_core();
    void run(int argc, char **argv);
    void updateTest();    
    void registerPlugin(std::unique_ptr<PluginInterface> plugin);
    void unregisterPlugin(std::unique_ptr<PluginInterface> plugin);
};


#endif