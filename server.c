#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#include "config.h"
#include "command.h"
#include "dataplane.h"

/********************************************
 * 监听用户指令线程
 ********************************************/



// 全局变量，指令队列和队列索引
Command commandQueue[SOCKET_QUEUE_SIZE];
int front = 0;
int rear = 0;

// 互斥锁和条件变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 线程函数，执行指令队列中的指令
void *executeCommands(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&mutex);
        // 检查指令队列是否为空
        while (front == rear)
        {
            // 指令队列为空，等待条件变量
            pthread_cond_wait(&cond, &mutex);
        }
        // 取出队列中的指令
        char command[SOCKET_BUFFER_SIZE];
        strcpy(command, commandQueue[front].command);
        // 更新队列索引
        front = (front + 1) % SOCKET_QUEUE_SIZE;
        pthread_mutex_unlock(&mutex);
        // 执行指令
        printf("Executing command: %s\n", command);
        system(command);
    }
    return NULL;
}

int listenCommand()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[SOCKET_BUFFER_SIZE] = {0};

    // 创建Socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // 设置Socket选项，允许地址重用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SOCKET_PORT);
    // 绑定Socket到指定端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // 监听连接
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // 接受并处理客户端连接
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // 读取客户端指令
        if ((valread = read(new_socket, buffer, SOCKET_BUFFER_SIZE)) > 0)
        {
            // 添加指令到队列
            pthread_mutex_lock(&mutex);
            if ((rear + 1) % SOCKET_QUEUE_SIZE == front)
            {
                // 队列已满，丢弃最早的指令
                front = (front + 1) % SOCKET_QUEUE_SIZE;
            }
            strcpy(commandQueue[rear].command, buffer);
            rear = (rear + 1) % SOCKET_QUEUE_SIZE;
            // 发送条件变量信号，唤醒等待的线程
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        // 关闭客户端的Socket文件描述符
        close(new_socket);
    }
    return 0;
}

int runserver()
{
    // 创建执行指令的线程
    pthread_t tid;
    if (pthread_create(&tid, NULL, executeCommands, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    listenCommand();
}
