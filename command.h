// 定义指令结构体
typedef struct
{
    char command[SOCKET_BUFFER_SIZE];
    int type;
} Command;