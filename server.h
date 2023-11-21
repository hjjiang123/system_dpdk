#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
/**
 * @brief Executes commands from a command queue in a separate thread.
 * 
 * This function is responsible for executing commands from a command queue in a separate thread.
 * It continuously checks if the command queue is empty and waits for a condition variable if it is.
 * Once a command is available in the queue, it retrieves the command, updates the queue index, and unlocks the mutex.
 * It then switches on the type of the command and performs the corresponding action.
 * 
 * @param arg A pointer to the argument passed to the thread function (not used in this case).
 * @return void* Always returns NULL.
 */
void *executeCommands(void *arg)
{
    printf("executeCommands\n");
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
        Command cur_cmd = commandQueue[front];
        // 更新队列索引
        front = (front + 1) % SOCKET_QUEUE_SIZE;
        pthread_mutex_unlock(&mutex);
        switch(cur_cmd.type){
            case REGISTER_PLUGIN:
                registerPlugin(cur_cmd.args.reg_plugin_arg);
                break;
            case UNREGISTE_RPLUGIN:
                unregisterPlugin(cur_cmd.args.unreg_plugin_arg.pluginid);
                break;
            case ADD_PLUGIN:
                push_Command(cur_cmd);
                break;
            case DELETE_PLUGIN:
                push_Command(cur_cmd);
                break;
            case ADD_FLOW_TO_QUEUE:
                addFlowToQueue(
                    cur_cmd.args.add_flow_arg.port_id,
                    cur_cmd.args.add_flow_arg.rx_q,
                    cur_cmd.args.add_flow_arg.src_ip,
                    cur_cmd.args.add_flow_arg.src_mask,
                    cur_cmd.args.add_flow_arg.dest_ip,
                    cur_cmd.args.add_flow_arg.dest_mask
                );
                break;
            case DELETE_FLOW_FROM_QUEUE:
                deleteFlowFromQueue(cur_cmd.args.del_flow_arg.id);
                break;
            case ADD_QUEUE_TO_CORE:
                push_Command(cur_cmd);
                break;
            case DELETE_QUEUE_FROM_CORE:
                push_Command(cur_cmd);
                break;
            default:
                break;
        }
        printf("Command executed\n");

    }
    return NULL;
}

/**
 * @brief This function listens for incoming commands from clients and processes them.
 * 
 * @param arg The argument passed to the function (not used in this case).
 * @return void* The return value is not used in this case.
 */
void* listenCommand(void *arg)
{
    printf("listenCommand\n");
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
    printf("Listening on port %d\n", SOCKET_PORT);
    // 接受并处理客户端连接
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted\n");
        // 读取客户端指令
        if ((valread = read(new_socket, buffer, SOCKET_BUFFER_SIZE)) > 0)
        {
            // 添加指令到队列
            printf("Command received\n");
            pthread_mutex_lock(&mutex);
            if ((rear + 1) % SOCKET_QUEUE_SIZE == front)
            {
                // 队列已满，丢弃最早的指令
                front = (front + 1) % SOCKET_QUEUE_SIZE;
            }
            Command temp_cmd;
            memcpy(&temp_cmd, buffer, sizeof(Command));
            commandQueue[rear] = temp_cmd;
            rear = (rear + 1) % SOCKET_QUEUE_SIZE;
            
            // 发送条件变量信号，唤醒等待的线程
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            printf("Command processed\n");
        }else{
            printf("Command error %d\n",valread);
        }
        // 关闭客户端的Socket文件描述符
        close(new_socket);
    }
    // return NULL;
}

/**
 * @brief Runs the server.
 * 
 * This function creates two threads to execute commands and listen for commands.
 * 
 * @return void
 */
int runserver()
{
    // 创建执行指令的线程
    pthread_t t1,t2;
    if (pthread_create(&t1, NULL, executeCommands, NULL) != 0)
    {
        perror("pthread_create 1");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&t2, NULL, listenCommand, NULL) != 0)
    {
        perror("pthread_create 2");
        exit(EXIT_FAILURE);
    }
    printf("runserver\n");
    return 0;
}
#endif