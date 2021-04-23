#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "library.h"

int serverQid, privateQid;
key_t serverKey, privateKey;

void handleINT(int sig) {
    exit(0);
}

void cleanUp() {
    msgctl(privateQid, IPC_RMID, (struct msqid_ds *) NULL);

    struct msgBuffer queryBuffer;
    queryBuffer.mtype = QUIT;

    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
}

int initClient() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    char *homeEnv = getenv("HOME");
    serverKey = ftok(homeEnv, SERVER_ID);
    if (serverKey == -1)
        return -1;
    privateKey = ftok(homeEnv, getpid());
    if (privateKey == -1)
        return -1;
    serverQid = msgget(serverKey, S_IRWXU | S_IRWXG);
    if (serverQid == -1)
        return -1;
    privateQid = msgget(privateKey, S_IRWXU | S_IRWXG | IPC_CREAT | IPC_EXCL);
    if (privateQid == -1)
        return -1;
    return 0;
}

int registerClient() {
    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;

    queryBuffer.mtype = REGISTERY;
    queryBuffer.key = privateKey;

    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1)
        return -1;

    errorCode = (int) msgrcv(privateQid, &responseBuffer, MSGBUF_RAW_SIZE, REGISTERY, 0);
    if (errorCode == -1)
        return -1;
    return responseBuffer.id;
}

int main(int argc, char **argv) {
    errorCode = initClient();
    if (errorCode == -1) {
        printf("%s 3\n", strerror(errno));
        exit(errno);
    }

    int myID = registerClient();
    if (myID == -1) {
        printf("%s 6\n", strerror(errno));
        exit(errno);
    }

}