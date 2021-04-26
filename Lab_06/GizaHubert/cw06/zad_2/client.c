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
    do {
        errorCode = mq_send(serverDesc, (char *) &queryBuffer, sizeof(struct msgBuffer), STOP);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
            errorCode = 1;
        }

    } while (errorCode == -1);


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
        if (errorCode == -1 && errno != EAGAIN) {
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

void handleCONNECT() {
    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;
    int val;
    queryBuffer.type = CONNECT;
    queryBuffer.id = myID;
    queryBuffer.intField = partnerID;
    char pathBuffer[PATH_MAX];
    sprintf(pathBuffer, "/%d", getpid());
    strcpy(queryBuffer.buffer, pathBuffer);
    do {
        errorCode = mq_send(serverDesc, (char *) &queryBuffer, MSGSIZE, CONNECT);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
        }
    } while (errorCode == -1);
    do {
        val = (int) mq_receive(myDesc, (char *) &responseBuffer, MSGSIZE, 0);
        if (val == -1 && errno != EAGAIN) {
            printf("tutaj %s\n", strerror(errno));
            raise(SIGINT);
        }
    } while (val < 0);
    printf("%s\n", responseBuffer.buffer);
    partnerDesc = mq_open(responseBuffer.buffer, O_WRONLY | O_NONBLOCK);
    if (partnerDesc == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    connected = 1;
}

int isConnect(char *line) {
    int n = (int) strlen(line);
    if (n < 10) {
        return 0;
    } else if (line[0] == 'C' && line[1] == 'O' && line[2] == 'N' && line[3] == 'N' && line[4] == 'E' &&
               line[5] == 'C' && line[6] == 'T' && line[7] == ' ' && line[8] != '\n') {
        int p = 8, q = 0;
        for (int i = 8; i < MAX_LINE_SIZE; i++) {
            if (line[i] >= '0' && line[i] <= '9') {
                continue;
            } else {
                q = i;
                break;
            }
        }
        char *charNumber = calloc(q - p + 1, sizeof(char));
        for (int i = p; i <= q; i++) {
            charNumber[i - p] = line[i];
        }
        charNumber[q - p] = '\0';
        int id = (int) atoi(charNumber);
        return id;
    }
    return -1;
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
            } else {
                if (strcmp(line, "LIST\n") == 0) {
                    errorCode = handleLIST();
                    if (errorCode == -1) {
                        raise(SIGINT);
                    }
                } else if ((partnerID = isConnect(line)) > 0) {
                    printf("Requesting connection with: %d\n", partnerID);
                    handleCONNECT();
                }
            }
        }
        errorCode = (int) mq_receive(myDesc, (char *) &guardBuffer, MSGSIZE, 0);
        if (errorCode == -1) {
            if (errorCode == -1 && errno != EAGAIN) {
                printf("%s\n", strerror(errno));
                printf("to tutaj\n");
            }
        } else {
            switch (guardBuffer.type) {
                case STOP:
                    printf("handling STOP\n");
                    printf("%s", guardBuffer.buffer);
                    raise(SIGINT);
                case CONNECT:
                    printf("handling CONNECT\n%s\n", guardBuffer.buffer);
                    partnerDesc = mq_open(guardBuffer.buffer, O_WRONLY | O_NONBLOCK);
                    if (partnerDesc == -1) {
                        printf("%s\n", strerror(errno));
                        raise(SIGINT);
                    }
                    struct msgBuffer queryBuffer;
                    char pathBuffer[PATH_MAX];
                    sprintf(pathBuffer, "/%d", getpid());
                    strcpy(queryBuffer.buffer, pathBuffer);
                    printf("%s\n", queryBuffer.buffer);
                    do {
                        errorCode = mq_send(serverDesc, (char *) &queryBuffer, MSGSIZE, CONNECT);
                        if (errorCode == -1 && errno != EAGAIN) {
                            printf("%s\n", strerror(errno));
                        }
                    } while (errorCode == -1);
                    printf("send descriptor to server \n");
                    connected = 1;
            }
        }
        nanosleep(&tim, &tim2);
    }
}

