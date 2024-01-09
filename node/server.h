#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include "dataplane.h"
#include "subtask_resolve.h"
/********************************************
 * 监听用户指令线程
 ********************************************/
/**
 * @brief Runs the server.
 * 
 * This function creates two threads to execute commands and listen for commands.
 * 
 * @return void
 */
int runserver();
#endif