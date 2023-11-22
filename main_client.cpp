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

    // add plugin
    Command cmd3;
    cmd3.type = ADD_PLUGIN;
    cmd3.args.add_plugin_arg.pluginid = 1;
    cmd3.args.add_plugin_arg.coreid = 10;
    sendCommand(cmd3);

    // unregister plugin
    // Command cmd2;
    // cmd2.type = UNREGISTE_RPLUGIN;
    // cmd2.args.unreg_plugin_arg.pluginid = 1;
    // sendCommand(cmd2);
    
    return 0;
}