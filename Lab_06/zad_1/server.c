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
int clientStatus[MAX_CLIENT_NUMBER];
int clientCounter = 0;

key_t serverKey;
int serverQid;

void cleanUp() {
    msgctl(serverQid, IPC_RMID, (struct msqid_ds *) NULL);
}

void handleINT(int sig) {
    printf("handling SIGINT\n");
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
    for (int i = 0; i < MAX_CLIENT_NUMBER; i++) {
        clientQids[i] = -1;
        clientStatus[i] = -1;
    }
    return 0;
}

int handleREGISTERY(struct msgBuffer queryBuffer, struct msgBuffer *responseBuffer) {
    clientQids[clientCounter] = msgget(queryBuffer.key, 0);
    if (clientQids[clientCounter] == -1) {
        return -1;
    }
    clientStatus[clientCounter] = 1;
    responseBuffer->mtype = REGISTRATION;
    responseBuffer->id = clientCounter;
    errorCode = msgsnd(msgget(queryBuffer.key, 0), responseBuffer, MSGBUF_RAW_SIZE, 0);

    if (errorCode == -1) {
        return -1;
    }
    clientCounter++;
    return 0;
}

int handleSTOP(struct msgBuffer queryBuffer) {
    printf("handling stop\n");
    if (msgctl(clientQids[queryBuffer.id], IPC_RMID, (struct msqid_ds *) NULL) == -1)
        return -1;
    clientQids[queryBuffer.id] = -1;
    clientCounter--;
    return 0;
}

void handleLIST(struct msgBuffer queryBuffer) {
    printf("handling LIST\n");
    struct msgBuffer respond;
    int any_active_clients = 0;
    for (int i = 0; i < clientCounter + 1; i++) {
        if (clientStatus[i] != -1) {
            respond.mtype = LIST;
            respond.id = i;
            errorCode = msgsnd(clientQids[queryBuffer.id], &respond, MSGBUF_RAW_SIZE, 0);
            if (errorCode == -1) {
                printf("%s\n", strerror(errno));
            } else {
                any_active_clients = 1;
            }
        }
    }
    if (any_active_clients) {
        respond.mtype = LIST;
        respond.id = -2137;
        msgsnd(clientQids[queryBuffer.id], &respond, MSGBUF_RAW_SIZE, 0);
    } else {
        respond.mtype = LIST;
        respond.id = -1;
        strcpy(respond.text, "There is no active users\n");
        msgsnd(clientQids[queryBuffer.id], &respond, MSGBUF_RAW_SIZE, 0);
    }
}

int main(int argc, char **argv) {
    errorCode = initServer();
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    printf("%d\n", serverKey);
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
            printf("Got query: %s - msg,\t %d - id,\t %lo - type\n",
                   queryBuffer.text == NULL ? "null" : queryBuffer.text, queryBuffer.id, queryBuffer.mtype);
            switch (queryBuffer.mtype) {
                case REGISTRATION:
                    errorCode = handleREGISTERY(queryBuffer, &responseBuffer);
                    if (errorCode == -1) {
                        printf("%s 2\n", strerror(errno));
                        return -1;
                    } else {
                        printf("registered with ID:%d\n", clientCounter - 1);
                    }
                    continue;
                case STOP:
                    handleSTOP(queryBuffer);
                    continue;
                case LIST:
                    handleLIST(queryBuffer);
                    continue;
            }
        }
    }
}