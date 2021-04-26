#include "library.h"

int serverQid, privateQid;
key_t serverKey, privateKey;
int myId;
int connected;
int partnerID, partnerQid;

void cleanUp() {
    printf("Cleaning up...\n");
    msgctl(privateQid, IPC_RMID, (struct msqid_ds *) NULL);

    struct msgBuffer queryBuffer;
    queryBuffer.mtype = STOP;
    queryBuffer.id = myId;
    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("couldnt send stop signal to server, but exiting anyway\n");
    }
    printf("Done!\n");
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

int handleLIST() {
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
            return -1;
        }
    }
    printf("End of list\n");
    return 1;
}

void handleCONNECT() {
    struct msgBuffer queryBuffer;
    queryBuffer.mtype = CONNECT;
    queryBuffer.id = myId;
    queryBuffer.key = partnerID;
    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
}

void handleDISCONNECT() {
    struct msgBuffer queryBuffer;
    queryBuffer.mtype = DISCONNECT;
    queryBuffer.id = myId;
    queryBuffer.key = partnerID;
    errorCode = msgsnd(serverQid, &queryBuffer, MSGBUF_RAW_SIZE, 0);
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    connected = 0;
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
    myId = responseBuffer.id;
    printf("You are now online with ID: %d\n", myId);
    return 1;
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
        printf("%s\n", strerror(errno));
        exit(errno);
    }
    errorCode = registerClient();
    if (errorCode == -1) {
        printf("%s\n", strerror(errno));
        exit(errno);
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
                if (strcmp(line, "DISCONNECT\n") == 0) {
                    printf("Quitting chat with partner %d\n", partnerID);
                    handleDISCONNECT();
                } else {
                    struct msgBuffer message;
                    message.mtype = MESSAGE;
                    message.id = myId;
                    strcpy(message.text, line);
                    errorCode = msgsnd(partnerQid, &message, MSGBUF_RAW_SIZE, 0);
                    if (errorCode == -1) {
                        printf("%s\n", strerror(errno));
                    }
                }

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
        errorCode = msgrcv(privateQid, &guardBuffer, MSGBUF_RAW_SIZE, 0, IPC_NOWAIT);
        if (errorCode == -1) {
            if (errno != ENOMSG) {
                printf("%s\n", strerror(errno));
            }
        } else {
            switch (guardBuffer.mtype) {
                case STOP:
                    printf("%s", guardBuffer.text);
                    raise(SIGINT);
                case CONNECT:
                    if (guardBuffer.id != -1) {
                        partnerQid = guardBuffer.id;
                        connected = 1;
                        printf("%s", guardBuffer.text);
                    }
                    guardBuffer.mtype = -1;
                    continue;
                case MESSAGE:
                    if (connected) {
                        printf("%d:\t %s", guardBuffer.id, guardBuffer.text);
                        guardBuffer.mtype = -1;
                    }
                    continue;
                case DISCONNECT:
                    printf("%s", guardBuffer.text);
                    connected = 0;
                    guardBuffer.mtype = -1;
                    continue;
            }
        }
        nanosleep(&tim, &tim2);
    }

}