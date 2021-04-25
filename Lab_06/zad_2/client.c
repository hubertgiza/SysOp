#include "library.h"

mqd_t myDesc;
mqd_t serverDesc;
int myID = -1;
int connected = 0;
int partnerID;
int partnerDesc;

void cleanUp() {
    mq_close(myDesc);

    struct msgBuffer queryBuffer;
    queryBuffer.type = STOP;
    queryBuffer.id = myID;
    mq_send(serverDesc, (char *) &queryBuffer, sizeof(struct msgBuffer), 0);

    mq_close(serverDesc);

    char pathBuffer[PATH_MAX];
    sprintf(pathBuffer, "/%d", getpid());
    mq_unlink(pathBuffer);

}

void handleINT(int sig) {
    exit(0);
}

int initClient() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    struct mq_attr attr;
    attr.mq_msgsize = MSGSIZE;
    attr.mq_maxmsg = MAX_MSG_NUMBER - 1;

    char pathBuffer[PATH_MAX];
    sprintf(pathBuffer, "/%d", getpid());

    serverDesc = mq_open(SERVER_PATH, O_WRONLY | O_NONBLOCK);
    if (serverDesc == -1)
        return -1;

    myDesc = mq_open(pathBuffer, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, PERMISSIONS, &attr);
    if (myDesc == -1)
        return -1;
    return 0;
}

int registerClient() {
    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;

    char pathBuffer[PATH_MAX];
    sprintf(pathBuffer, "/%d", getpid());

    queryBuffer.type = REGISTRATION;
    strcpy(queryBuffer.buffer, pathBuffer);

    do {
        errorCode = mq_send(serverDesc, (char *) &queryBuffer, MSGSIZE, queryBuffer.type);
        if (errorCode == -1 && errno != EAGAIN){
            printf("%s\n", strerror(errno));
            return -1;
        }

    } while (errorCode == -1);

    do {
        errorCode = (int) mq_receive(myDesc, (char *) &responseBuffer, sizeof(struct msgBuffer), NULL);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
            return -1;
        }
    } while (errorCode == -1);

    myID = responseBuffer.id;
    printf("You are now online with ID: %d\n", myID);
    return 0;
}

int handleLIST() {
    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;
    queryBuffer.id = myID;
    queryBuffer.type = LIST;
    do {
        errorCode = mq_send(serverDesc, (char *) &queryBuffer, MSGSIZE, queryBuffer.type);
        if (errorCode == -1 && errno != EAGAIN)
            return -1;
    } while (errorCode == -1);
    do {
        errorCode = (int) mq_receive(myDesc, (char *) &responseBuffer, sizeof(struct msgBuffer), NULL);
        if (errorCode == -1 && errno != EAGAIN)
            return -1;
    } while (errorCode == -1);
    printf("List of active users\n%sEnd of list\n", responseBuffer.buffer);
    return 0;
}

int main(int argc, char **argv) {
    errorCode = initClient();
    if (errorCode == -1) {
        printf("%s 1\n", strerror(errno));
        return 1;
    }
    errorCode = registerClient();
    if (errorCode == -1) {
        printf("%d %s 2\n", errno, strerror(errno));
        return 1;
    }

    while (1) {
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = 200000000;
        struct msgBuffer guardBuffer;
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
            if (connected) {
//                if (strcmp(line, "DISCONNECT\n") == 0) {
//                    printf("Quitting chat with partner %d\n", partnerID);
//                    handleDISCONNECT();
//                } else {
//                    struct msgBuffer message;
//                    message.mtype = MESSAGE;
//                    message.id = myId;
//                    strcpy(message.text, line);
//                    errorCode = msgsnd(partnerQid, &message, MSGBUF_RAW_SIZE, 0);
//                    if (errorCode == -1) {
//                        printf("%s\n", strerror(errno));
//                    }
//                }
            } else {
                if (strcmp(line, "LIST\n") == 0) {
                    errorCode = handleLIST();
                    if (errorCode == -1) {
                        raise(SIGINT);
                    }
                }
//                else if ((partnerID = isConnect(line)) > 0) {
//                    printf("Requesting connection with: %d\n", partnerID);
//                    handleCONNECT();
//                }
            }
        }

//        errorCode = msgrcv(privateQid, &guardBuffer, MSGBUF_RAW_SIZE, 0, IPC_NOWAIT);
//        if (errorCode == -1) {
//            if (errno != ENOMSG) {
//                printf("%s s\n", strerror(errno));
//            }
//        } else {
//            switch (guardBuffer.mtype) {
//                case STOP:
//                    printf("%s", guardBuffer.text);
//                    raise(SIGINT);
//                case CONNECT:
//                    if (guardBuffer.id != -1) {
//                        partnerQid = guardBuffer.id;
//                        connected = 1;
//                        printf("%s", guardBuffer.text);
//                    }
//                    guardBuffer.mtype = -1;
//                    continue;
//                case MESSAGE:
//                    if (connected) {
//                        printf("%d:\t %s", guardBuffer.id, guardBuffer.text);
//                        guardBuffer.mtype = -1;
//                    }
//                    continue;
//                case DISCONNECT:
//                    printf("%s", guardBuffer.text);
//                    connected = 0;
//                    guardBuffer.mtype = -1;
//                    continue;
//            }
//        }
        nanosleep(&tim, &tim2);
    }
}

