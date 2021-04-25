//
// Created by Hubert Giza on 25/04/2021.
//

#ifndef SYSOP_COMMON_H
#define SYSOP_COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>

#define SERVER_PATH "/server_queue"

#define MAX_LINE_SIZE 512
#define MAX_CLIENT_NUMBER 1024
#define MAX_MSG_NUMBER 10

struct msgBuffer {
    long type;
    int id;
    char buffer[MAX_LINE_SIZE];
};

enum operationType {
    STOP = 1,
    DISCONNECT = 2,
    REGISTRATION = 3,
    LIST = 4,
    CONNECT = 5,
    MESSAGE = 6
};

int errorCode;
#endif //SYSOP_COMMON_H
