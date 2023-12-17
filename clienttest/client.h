#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "../node/config.h"
#include "../node/command.h"


/**
 * @brief Sends a command to the server.
 * 
 * This function creates a socket, connects to the server, and sends a command to it.
 * The command is serialized and sent as a string over the socket.
 * 
 * @param cmd The command to be sent.
 * @return 0 if the command was sent successfully, -1 otherwise.
 */
int sendCommand(Command cmd) {
    
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[SOCKET_BUFFER_SIZE] = {0};
    
    // 创建Socket文件描述符
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SOCKET_PORT);
    
    // 将IP地址从点分十进制转换为二进制格式
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    // 连接到服务端
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    // serializeCommand(cmd, buffer);
    // 读取用户输入的指令并发送到服务端
    send(sock, &cmd, sizeof(Command), 0);
    printf("command type:%d sent\n",cmd.type);
    // 关闭客户端的Socket文件描述符
    close(sock);
    return 0;
}


#endif