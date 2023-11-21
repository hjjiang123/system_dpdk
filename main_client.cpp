#include "client.h"

int main(){
    Command cmd={
        .type=REGISTER_PLUGIN,
        .args.reg_plugin_arg={
            .id=0,
            .cnt_info={
                .rownum=1,
                .bucketnum=1024,
                .bucketsize=4,
                .countersize=4,
            },
            .hash_info={
                .hashnum=1,
                .key_len=4,
                .entries=1024,
            },
            .funcname="process",
            .filename="test1.so",
        }
    };
    sendCommand(cmd);
    return 0;
}