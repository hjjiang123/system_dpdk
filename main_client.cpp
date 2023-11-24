#include "client.h"

int main(){
    // register plugin
    Command cmd1;
    cmd1.type = REGISTER_PLUGIN;
    cmd1.args.reg_plugin_arg.id = 0;
    cmd1.args.reg_plugin_arg.cnt_info.rownum = 1;
    cmd1.args.reg_plugin_arg.cnt_info.bucketnum = 1024;
    cmd1.args.reg_plugin_arg.cnt_info.bucketsize = 4;
    cmd1.args.reg_plugin_arg.cnt_info.countersize = 4;
    cmd1.args.reg_plugin_arg.hash_info.hashnum = 1;
    cmd1.args.reg_plugin_arg.hash_info.key_len = 4;
    cmd1.args.reg_plugin_arg.hash_info.entries = 1024;
    strncpy(cmd1.args.reg_plugin_arg.funcname, "process", sizeof(cmd1.args.reg_plugin_arg.funcname) - 1);
    strncpy(cmd1.args.reg_plugin_arg.filename, "test1.so", sizeof(cmd1.args.reg_plugin_arg.filename) - 1);
    sendCommand(cmd1);
    Command cmd7;
    cmd7.type = ADD_QUEUE_TO_CORE;
    cmd7.args.add_queue_arg.queueid = 0;
    cmd7.args.del_plugin_arg.coreid = 10;
    sendCommand(cmd7);
    sleep(1);
    Command cmd3;
    cmd3.type = ADD_PLUGIN;
    cmd3.args.add_plugin_arg.pluginid = 1;
    cmd3.args.add_plugin_arg.coreid = 10;
    sendCommand(cmd3);
    // sleep(1);
    // Command cmd4;
    // cmd4.type = DELETE_PLUGIN;
    // cmd4.args.del_plugin_arg.pluginid = 1;
    // cmd4.args.del_plugin_arg.coreid = 10;
    // sendCommand(cmd4);

    // Command cmd5;
    // cmd5.type = ADD_FLOW_TO_QUEUE;
    // cmd5.args.add_flow_arg.src_ip = inet_addr("211.65.0.0");
    // cmd5.args.add_flow_arg.src_mask = inet_addr("255.255.0.0");
    // cmd5.args.add_flow_arg.dest_ip = inet_addr("0.0.0.0");
    // cmd5.args.add_flow_arg.dest_mask = inet_addr("0.0.0.0");
    // cmd5.args.add_flow_arg.port_id = 0;
    // cmd5.args.add_flow_arg.rx_q = 0;
    // sendCommand(cmd5);

    

    // unregister plugin
    // Command cmd2;
    // cmd2.type = UNREGISTE_RPLUGIN;
    // cmd2.args.unreg_plugin_arg.pluginid = 1;
    // sendCommand(cmd2);
    
    return 0;
}