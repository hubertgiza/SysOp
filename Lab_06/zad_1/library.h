#ifndef SYSOP_LIBRARY_H
#define SYSOP_LIBRARY_H

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define MAX_CLIENT_NUMBER 128
#define MAX_LINE_SIZE 64

#define SERVER_ID 's'
#define PERMISSIONS 0660
enum operationType {
    REGISTRATION = 1,
    STOP = 2,
    LIST = 3
};


struct msgBuffer {
    long mtype;
    int id;
    key_t key;
    char text[MAX_LINE_SIZE];
};

const size_t MSGBUF_RAW_SIZE = sizeof(struct msgBuffer) - sizeof(long);

struct clientInfo {
    int qid;
};

int errorCode;
#endif //SYSOP_LIBRARY_H
