//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/server.c
//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/common.h
//https://github.com/SirSpectacular/Sysopy/blob/master/Zestaw-06/zad2/client.c
#include "library.h"

mqd_t serverDesc;
mqd_t clientDescs[MAX_CLIENT_NUMBER];
int clientCounter = 0;

int initServer() {
    atexit(cleanUp);
    signal(SIGINT, handleINT);

    struct mq_attr attr;
    attr.mq_msgsize = sizeof(struct msgBuffer);
    attr.mq_maxmsg = 10;

    return mq_open(SERVER_PATH, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, S_IRWXU | S_IRWXG, &attr);
}

void cleanUp() {
    mq_unlink(SERVER_PATH);
    mq_close(serverDesc);
    for (int i = 0; i < clientCounter; i++)
        mq_close(clientDescs[i]);
}

void handleINT(int sig) {
    exit(0);
}