#include "library.h"


int clientQids[MAX_CLIENT_NUMBER];
int clientStatus[MAX_CLIENT_NUMBER];
int idCounter = 0;
int activeClients = 0;

key_t serverKey;
int serverQid;

void cleanUp() {
    msgctl(serverQid, IPC_RMID, (struct msqid_ds *) NULL);
    printf("Server has gone offline\n");
}

void handleINT(int sig) {
    int signals_send = 0;
    for (int i = 0; i < idCounter; i++) {
        if (clientStatus[i] != -1) {
            struct msgBuffer queryBuffer;
            struct msgBuffer responseBuffer;
            queryBuffer.mtype = STOP;
            strcpy(queryBuffer.text, "Server is shutting down\nYou will be disconnected\n\0");
            printf("Send STOP to client: %d \n", i);
            msgsnd(clientQids[i], &queryBuffer, MSGBUF_RAW_SIZE, 0);
            msgrcv(serverQid, &responseBuffer, MSGBUF_RAW_SIZE, 0, 0);
            if (responseBuffer.mtype == STOP) {
                signals_send++;
            }
            if (signals_send == activeClients) {
                break;
            }
        }
    }
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
    printf("Server is online\n");
    return 0;
}

int handleREGISTERY(struct msgBuffer queryBuffer, struct msgBuffer *responseBuffer) {
    clientQids[idCounter] = msgget(queryBuffer.key, 0);
    if (clientQids[idCounter] == -1) {
        return -1;
    }
    clientStatus[idCounter] = 1;
    responseBuffer->mtype = REGISTRATION;
    responseBuffer->id = idCounter;
    errorCode = msgsnd(msgget(queryBuffer.key, 0), responseBuffer, MSGBUF_RAW_SIZE, 0);

    if (errorCode == -1) {
        return -1;
    }
    printf("Registered with ID:%d\n", idCounter);
    idCounter++;
    activeClients++;
    return 0;
}

int handleSTOP(struct msgBuffer queryBuffer) {
    clientQids[queryBuffer.id] = -1;
    clientStatus[queryBuffer.id] = -1;
    activeClients--;
    return 0;
}

int handleCONNECT(struct msgBuffer queryBuffer) {
    int client_1 = queryBuffer.id;
    int client_2 = queryBuffer.key;
    int errorCode1;
    int errorCode2;
    struct msgBuffer signalBuffer;
    signalBuffer.mtype = CONNECT;
    signalBuffer.id = clientQids[client_2];
    strcpy(signalBuffer.text, "Server: You are now in chat!\n\0");
    errorCode1 = msgsnd(clientQids[client_1], &signalBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode1 == -1) {
        printf("%s\n", strerror(errno));
    }
    signalBuffer.id = clientQids[client_1];
    errorCode2 = msgsnd(clientQids[client_2], &signalBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode2 == -1) {
        printf("%s\n", strerror(errno));
    }
    if (errorCode1 != -1 && errorCode2 != -1) {
        clientStatus[client_1] = 0;
        clientStatus[client_2] = 0;
        return 1;
    } else {
        return -1;
    }
}

void handleLIST(struct msgBuffer queryBuffer) {
    struct msgBuffer respond;
    int any_active_clients = 0;
    for (int i = 0; i < idCounter; i++) {
        if (clientStatus[i] == 1) {
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

void handleDISCONNECT(struct msgBuffer queryBuffer) {
    struct msgBuffer responseBuffer;
    char line[MAX_LINE_SIZE] = {"You have been disconnected from chat\nYou are online on server again\n\0"};
    responseBuffer.mtype = DISCONNECT;
    strcpy(responseBuffer.text, line);
    errorCode = msgsnd(clientQids[queryBuffer.id], &responseBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
    } else {
        clientStatus[queryBuffer.id] = 1;
    }

    errorCode = msgsnd(clientQids[queryBuffer.key], &responseBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
    } else {
        clientStatus[queryBuffer.key] = 1;
    }

}

int main(int argc, char **argv) {
    errorCode = initServer();
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    while (1) {
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = 100000000;
        struct msgBuffer queryBuffer;
        struct msgBuffer responseBuffer;
        errorCode = (int) msgrcv(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0, IPC_NOWAIT);
        if (errorCode == -1) {
            if (errno != ENOMSG) {
                printf("%s s\n", strerror(errno));
                return 1;
            }
        } else {
            printf("Got query %d - id,\t %lo - type\n", queryBuffer.id, queryBuffer.mtype);
            switch (queryBuffer.mtype) {
                case REGISTRATION:
                    errorCode = handleREGISTERY(queryBuffer, &responseBuffer);
                    if (errorCode == -1) {
                        printf("%s 2\n", strerror(errno));
                        return -1;
                    }
                    continue;
                case STOP:
                    handleSTOP(queryBuffer);
                    continue;
                case LIST:
                    handleLIST(queryBuffer);
                    continue;
                case CONNECT:
                    handleCONNECT(queryBuffer);
                    continue;
                case DISCONNECT:
                    handleDISCONNECT(queryBuffer);
                    continue;
            }
        }
        nanosleep(&tim, &tim2);
    }
}