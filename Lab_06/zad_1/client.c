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
int myId;

void cleanUp() {
    printf("cleaning up\n");
    msgctl(privateQid, IPC_RMID, (struct msqid_ds *) NULL);

    struct msgBuffer queryBuffer;
    queryBuffer.mtype = STOP;
    queryBuffer.id = myId;
    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("couldnt send stop signal to server, but exiting anyway\n");
    }
}

void handleINT(int sig) {
    exit(0);
}

int initClient() {


    serverKey = ftok(getenv("HOME"), SERVER_ID);
    if (serverKey == -1)
        return -1;

    privateKey = ftok(getenv("HOME"), getpid());
    if (privateKey == -1)
        return -1;

    serverQid = msgget(serverKey, 0);
    if (serverQid == -1) {

        return -1;
    }


    privateQid = msgget(privateKey, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if (privateQid == -1) {

        return -1;
    }

    atexit(cleanUp);
    signal(SIGINT, handleINT);
    return 0;
}

int registerClient() {
    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;

    queryBuffer.mtype = REGISTRATION;
    queryBuffer.key = privateKey;

    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        return -1;
    }

    errorCode = (int) msgrcv(privateQid, &responseBuffer, MSGBUF_RAW_SIZE, REGISTRATION, 0);
    if (errorCode == -1) {
        return -1;
    }

    return responseBuffer.id;
}

int main(int argc, char **argv) {
    errorCode = initClient();
    if (errorCode == -1) {
        printf("%s 3\n", strerror(errno));
        exit(errno);
    }
    myId = registerClient();
    if (myId == -1) {
        printf("%s 6\n", strerror(errno));
        exit(errno);
    }
    while (1) {
        fd_set rfds;
        struct timeval tv;
        int retval;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
            perror("select()");
        else if (retval) {
            int i = 0;
            char line[MAX_LINE_SIZE];
            char buff;
            while (read(STDIN_FILENO, &buff, 1) >= 0) {
                line[i] = buff;
                i++;
                if (buff == '\n')
                    break;
            }
            line[i] = '\0';
            if (strcmp(line, "LIST\n") == 0) {
                printf("%d\n", privateQid);
                struct msgBuffer queryBuffer;
                struct msgBuffer responseBuffer;
                int read = 1;
                queryBuffer.mtype = LIST;
                queryBuffer.id = myId;
                msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
                printf("List of active users:\n");
                while (read) {
                    msgrcv(privateQid, &responseBuffer, MSGBUF_RAW_SIZE, LIST, 0);
                    if (responseBuffer.id >= 0) {
                        printf("%d\n", responseBuffer.id);
                    } else if (responseBuffer.id == -2137) {
                        read = 0;
                    } else if (responseBuffer.id == -1) {
                        printf("%s", responseBuffer.text);
                        read = 0;
                    } else {
                        printf("cos sie popsulo %d\n", responseBuffer.id);
                        read = 0;
                    }
                }
                printf("end of LIST\n");
            }
        }
    }

}