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
    mq_unlink(SERVER_PATH);
    mq_close(serverDesc);
    for(int i=0;i<idCounter;i++){
        if(clientStatus[i]!=-1){
            do {
                errorCode = mq_send(clientDescs[i], (char *) &queryBuffer, sizeof(struct msgBuffer),STOP);
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

int handleREGISTERY(struct msgBuffer queryBuffer, struct msgBuffer *responseBuffer) {
    clientDescs[idCounter] = mq_open(queryBuffer.buffer, O_WRONLY | O_NONBLOCK);
    if (clientDescs[idCounter] == -1)
        return -1;
    clientStatus[idCounter]=1;
    responseBuffer->id = idCounter;
    printf("Registered with ID: %d\n",idCounter);
    idCounter++;
    activeClients++;
    return 1;
}

int handleSTOP(struct msgBuffer queryBuffer) {
    if (mq_close(clientDescs[queryBuffer.id]) == -1)
        return -1;
    activeClients--;
    return 0;
}

int handleLIST(struct msgBuffer queryBuffer,struct msgBuffer *responseBuffer) {
    char line[MAX_LINE_SIZE];
    int k = 0;
    for (int i = 0; i < idCounter; i++) {
        if(clientStatus[i]==1){
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
    strcpy(responseBuffer->buffer,line);
    return 0;
}

int initServer() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    struct mq_attr attr;
    attr.mq_msgsize = MSGSIZE;
    attr.mq_maxmsg = MAX_MSG_NUMBER;
    for(int i=0;i<MAX_CLIENT_NUMBER;i++){
        clientDescs[i]=-1;
        clientStatus[i]=-1;
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

    struct msgBuffer queryBuffer;
    struct msgBuffer responseBuffer;

    while (1) {
        errorCode = (int) mq_receive(serverDesc, (char *) &queryBuffer, sizeof(queryBuffer), 0);
        if (errorCode == -1) {
            if (errno != EAGAIN) {
                printf("%s\n", strerror(errno));
                return -1;
            }
        } else {
            printf("Got query %d - id %lo - type\n", queryBuffer.id, queryBuffer.type);
            switch (queryBuffer.type) {
                case REGISTRATION:
                    errorCode = handleREGISTERY(queryBuffer, &responseBuffer);
                    if (errorCode == -1) {
                        printf("%s\n", strerror(errno));
                        return -1;
                    }
                    break;
                case STOP:
                    errorCode = handleSTOP(queryBuffer);
                    if (errorCode == -1) {
                        printf("%s\n", strerror(errno));
                        return -1;
                    }
                    continue;
                case LIST:
                    errorCode = handleLIST(queryBuffer,&responseBuffer);
                    break;
            }
            responseBuffer.type = queryBuffer.type;
            do {
                errorCode = mq_send(clientDescs[responseBuffer.id], (char *) &responseBuffer, sizeof(struct msgBuffer),
                                    0);
                if (errorCode == -1 && errno != EAGAIN) {
                    printf("%s\n", strerror(errno));
                    return -1;
                }
            } while (errorCode == -1);
        }
    }
}

