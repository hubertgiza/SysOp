#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "library.h"

#define SERVER_ID 's'
#define MAX_CLIENT_NUMBER 1024
#define MAX_LINE_SIZE 512

int clientQids[MAX_CLIENT_NUMBER];
int clientCounter = 0;

key_t serverKey;
int serverQid;

void cleanUp() {
    msgctl(serverQid, IPC_RMID, (struct msqid_ds *) NULL);
}

void handleINT(int sig) {
    exit(0);
}

int initServer() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    serverKey = ftok(getenv("HOME"), SERVER_ID);
    if (serverKey == -1)
        return -1;

    serverQid = msgget(serverKey, IPC_CREAT | PERMISSIONS);
    if (serverQid == -1)
        return -1;
    return 0;
}

int main(int argc, char **argv) {
    errorCode = initServer();
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    while (1) {
        struct msgBuffer queryBuffer;
        struct msgBuffer responseBuffer;
        int readyToExit = 0;
        errorCode = (int) msgrcv(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0, IPC_NOWAIT);
        if (errorCode == -1) {
            if (errno != ENOMSG) {
                printf("%s s\n", strerror(errno));
                return 1;
            }
        } else {
            printf("Got query: %s - msg,\t %d - id,\t %lo - type\n", queryBuffer.buffer == NULL ? "null" : queryBuffer.buffer, queryBuffer.id, queryBuffer.mtype);
        }
    }
}