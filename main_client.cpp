#include "client.h"

int main(){
    Command cmd;
    cmd.type = REGISTER_PLUGIN;
    cmd.args.reg_plugin_arg.id = 0;
    cmd.args.reg_plugin_arg.cnt_info.rownum = 1;
    cmd.args.reg_plugin_arg.cnt_info.bucketnum = 1024;
    cmd.args.reg_plugin_arg.cnt_info.bucketsize = 4;
    cmd.args.reg_plugin_arg.cnt_info.countersize = 4;
    cmd.args.reg_plugin_arg.hash_info.hashnum = 1;
    cmd.args.reg_plugin_arg.hash_info.key_len = 4;
    cmd.args.reg_plugin_arg.hash_info.entries = 1024;
    strncpy(cmd.args.reg_plugin_arg.funcname, "process", sizeof(cmd.args.reg_plugin_arg.funcname) - 1);
    strncpy(cmd.args.reg_plugin_arg.filename, "test1.so", sizeof(cmd.args.reg_plugin_arg.filename) - 1);
    sendCommand(cmd);
    return 0;
}