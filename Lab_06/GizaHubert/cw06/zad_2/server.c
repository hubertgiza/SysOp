//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/server.c
//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/common.h
//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/client.c
#include "library.h"

mqd_t serverDesc;
mqd_t clientDescs[MAX_CLIENT_NUMBER];
int clientStatus[MAX_MSG_NUMBER];
int idCounter = 0;
int activeClients = 0;

void cleanUp() {
    struct msgBuffer queryBuffer;
    queryBuffer.type = STOP;
    strcpy(queryBuffer.buffer, "Server is shutting down\nYou will be disconnected\n\0");
    mq_unlink(SERVER_PATH);
    mq_close(serverDesc);
    for (int i = 0; i < idCounter; i++) {
        if (clientStatus[i] != -1) {
            printf("Send STOP to client: %d \n", i);
            do {
                errorCode = mq_send(clientDescs[i], (char *) &queryBuffer, MSGSIZE, STOP);
                if (errorCode == -1 && errno != EAGAIN) {
                    printf("%s\n", strerror(errno));
                }
            } while (errorCode == -1);
            mq_close(clientDescs[i]);
        }
    }
}

void handleINT(int sig) {
    exit(0);
}

int handleREGISTRATION(struct msgBuffer queryBuffer) {
    struct msgBuffer responseBuffer;
    printf("%s\n", queryBuffer.buffer);
    clientDescs[idCounter] = mq_open(queryBuffer.buffer, O_WRONLY | O_NONBLOCK);
    if (clientDescs[idCounter] == -1)
        return -1;
    responseBuffer.type = REGISTRATION;
    responseBuffer.id = idCounter;
    do {
        errorCode = mq_send(clientDescs[responseBuffer.id], (char *) &responseBuffer, MSGSIZE, responseBuffer.type);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
            return -1;
        }
    } while (errorCode == -1);
    clientStatus[idCounter] = 1;
    printf("Registered with ID: %d\n", idCounter);
    idCounter++;
    activeClients++;
    return 1;
}

void handleSTOP(struct msgBuffer queryBuffer) {
    if (mq_close(clientDescs[queryBuffer.id]) == -1) {
        printf("Could not close clients query, but server continues to work\n");
        printf("%s\n", strerror(errno));
    }
    clientStatus[queryBuffer.id] = -1;
    activeClients--;
}

void handleLIST(struct msgBuffer queryBuffer) {
    struct msgBuffer responseBuffer;
    char line[MAX_LINE_SIZE];
    int k = 0;
    for (int i = 0; i < idCounter; i++) {
        if (clientStatus[i] == 1) {
            if (i < 10) {
                line[k] = i + 48;
                line[k + 1] = '\n';
                k += 2;
            } else if (i < 100) {
                line[k] = i / 10 + 48;
                line[k + 1] = i % 10 + 48;
                line[k + 2] = '\n';
                k += 3;
            }
        }
    }
    line[k] = '\0';
    responseBuffer.type = LIST;
    strcpy(responseBuffer.buffer, line);
    do {
        errorCode = mq_send(clientDescs[queryBuffer.id], (char *) &responseBuffer, MSGSIZE, responseBuffer.type);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
            break;
        }
    } while (errorCode == -1);

}

void handleDISCONNECT(struct msgBuffer queryBuffer) {

}

void handleCONNECT(struct msgBuffer queryBuffer) {
    struct msgBuffer responseBuffer;
    struct msgBuffer answerBuffer;
    int val;
    int client1 = queryBuffer.id;
    int client2 = queryBuffer.intField;
    responseBuffer.type = CONNECT;
    printf("%s\n",queryBuffer.buffer);
    strcpy(responseBuffer.buffer, queryBuffer.buffer);
    do {
        errorCode = mq_send(clientDescs[client2], (char *) &responseBuffer, MSGSIZE, CONNECT);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
        }
    } while (errorCode == -1);
    printf("send to client 2\n");
    do {
        val = (int) mq_receive(serverDesc, (char *) &answerBuffer, MSGSIZE, 0);
        if (val == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
        }
    } while (val <= 0);
    printf("received from client\n");

    strcpy(responseBuffer.buffer, answerBuffer.buffer);
    printf("%s\n", answerBuffer.buffer);
    do {
        errorCode = mq_send(clientDescs[client1], (char *) &responseBuffer, MSGSIZE, CONNECT);
        if (errorCode == -1 && errno != EAGAIN) {
            printf("%s\n", strerror(errno));
        }
    } while (errorCode == -1);
    printf("send to client 1\n");
    clientDescs[client1] = 0;
    clientDescs[client2] = 0;
}

int initServer() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    struct mq_attr attr;
    attr.mq_msgsize = MSGSIZE;
    attr.mq_maxmsg = MAX_MSG_NUMBER;
    for (int i = 0; i < MAX_CLIENT_NUMBER; i++) {
        clientDescs[i] = -1;
        clientStatus[i] = -1;
    }
    printf("Server is online\n");
    return mq_open(SERVER_PATH, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, PERMISSIONS, &attr);
}

int main(int argc, char **argv) {
    serverDesc = initServer();
    if (serverDesc == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 100000000;
    while (1) {
        struct msgBuffer queryBuffer;
        errorCode = (int) mq_receive(serverDesc, (char *) &queryBuffer, MSGSIZE, 0);
        if (errorCode == -1) {
            if (errno != EAGAIN) {
                printf("%s\n", strerror(errno));
                return -1;
            }
        } else {
            printf("Got query %d - id %lo - type\n", queryBuffer.id, queryBuffer.type);
            switch (queryBuffer.type) {
                case STOP:
                    handleSTOP(queryBuffer);
                    break;
                case DISCONNECT:
                    handleDISCONNECT(queryBuffer);
                    break;
                case REGISTRATION:
                    handleREGISTRATION(queryBuffer);
                    break;
                case CONNECT:
                    handleCONNECT(queryBuffer);
                    break;
                case LIST:
                    handleLIST(queryBuffer);
                    break;
            }
        }
        nanosleep(&tim, &tim2);
    }
}

