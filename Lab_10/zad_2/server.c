#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PLAYERS 20
#define MAX_BACKLOG 10
#define MAX_MESSAGE_LENGTH 256

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
    char *name;
    int descriptor;
    int online;
    struct sockaddr addr;
} client;

client *clients[MAX_PLAYERS] = {NULL};
int clients_count = 0;


int add_client(char *name, struct sockaddr addr, int fd) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
            return -1;
        }
    }
    int index = -1;
    for (int i = 0; i < MAX_PLAYERS; i += 2) {
        if (clients[i] != NULL && clients[i + 1] == NULL) {
            index = i + 1;
            break;
        }
    }
    if (index == -1) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clients[i] == NULL) {
                index = i;
                break;
            }
        }
    }

    if (index != -1) {
        client *new_client = calloc(1, sizeof(client));
        new_client->name = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
        strcpy(new_client->name, name);
        new_client->descriptor = fd;
        new_client->online = 1;
        new_client->addr = addr;
        clients[index] = new_client;
        clients_count++;
    }

    return index;
}


int find_opponent(int index) {
    if (index % 2 == 0)
        return index + 1;
    else
        return index - 1;
}

void remove_client(char *name) {
    int index = -1;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
            index = i;
        }
    }
    printf("Removing client: %s\n", name);
    free(clients[index]->name);
    free(clients[index]);
    clients[index] = NULL;
    clients_count--;
    int rival = find_opponent(index);
    if (clients[rival] != NULL) {
        printf("Removing rival: %s\n", clients[rival]->name);
        sendto(clients[rival]->descriptor, "quit: ", MAX_MESSAGE_LENGTH, 0,
               &clients[rival]->addr, sizeof(struct addrinfo));
        free(clients[rival]->name);
        free(clients[rival]);
        clients[rival] = NULL;
        clients_count--;
    }
}

void *ping() {
    while (1) {
        printf("*PINGING*\n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clients[i] != NULL && !clients[i]->online) {
                remove_client(clients[i]->name);
            }
        }
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clients[i] != NULL) {
                sendto(clients[i]->descriptor, "ping: ", MAX_MESSAGE_LENGTH, 0, &clients[i]->addr,
                       sizeof(struct addrinfo));
                clients[i]->online = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(3);
    }
    return NULL;
}

int init_local_socket(char *path) {
    int local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un local_sockaddr;
    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, path);
    unlink(path);
    bind(local_socket, (struct sockaddr *) &local_sockaddr, sizeof(struct sockaddr_un));
    listen(local_socket, MAX_BACKLOG);
    return local_socket;
}

int init_network_socket(char *port) {
    struct addrinfo *info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &info);
    int network_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    bind(network_socket, info->ai_addr, info->ai_addrlen);
    listen(network_socket, MAX_BACKLOG);
    freeaddrinfo(info);
    return network_socket;
}

int check_messages(int local_socket, int network_socket) {
    struct pollfd fds[2];
    fds[0].fd = local_socket;
    fds[0].events = POLLIN;
    fds[1].fd = network_socket;
    fds[1].events = POLLIN;
    poll(fds, 2, -1);
    for (int i = 0; i < 2; i++) {
        if (fds[i].revents & POLLIN) {
            return fds[i].fd;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments. Got %d, expected 2\n", argc - 1);
    }
    char *port = argv[1];
    char *socket_path = argv[2];
    srand(time(NULL));
    int local_socket = init_local_socket(socket_path);
    int network_socket = init_network_socket(port);

    pthread_t t;
    pthread_create(&t, NULL, ping, NULL);
    while (1) {
        int socket_fd = check_messages(local_socket, network_socket);
        char buffer[MAX_MESSAGE_LENGTH + 1];
        struct sockaddr from_addr;
        socklen_t from_length = sizeof(struct sockaddr);
        recvfrom(socket_fd, buffer, MAX_MESSAGE_LENGTH, 0, &from_addr, &from_length);
        printf("%s\n", buffer);
        char *command = strtok(buffer, ":");
        char *arg = strtok(NULL, ":");
        char *name = strtok(NULL, ":");
        pthread_mutex_lock(&mutex);
        if (strcmp(command, "add") == 0) {
            int index = add_client(name, from_addr, socket_fd);
            if (index == -1) {
                sendto(socket_fd, "add:name_taken", MAX_MESSAGE_LENGTH, 0, (struct sockaddr *) &from_addr,
                       sizeof(struct addrinfo));
            } else if (index % 2 == 0) {
                sendto(socket_fd, "add:no_enemy", MAX_MESSAGE_LENGTH, 0, (struct sockaddr *) &from_addr,
                       sizeof(struct addrinfo));
            } else {
                int random_num = rand() % 100;
                int first, second;
                if (random_num % 2 == 0) {
                    first = index;
                    second = find_opponent(index);
                } else {
                    second = index;
                    first = find_opponent(index);
                }
                sendto(clients[first]->descriptor, "add:O", MAX_MESSAGE_LENGTH, 0, &clients[first]->addr,
                       sizeof(struct addrinfo));
                sendto(clients[second]->descriptor, "add:X", MAX_MESSAGE_LENGTH, 0, &clients[second]->addr,
                       sizeof(struct addrinfo));
            }
        } else if (strcmp(command, "move") == 0) {
            int move = atoi(arg);
            int player = -1;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
                    player = i;
                }
            }
            sprintf(buffer, "move:%d", move);
            sendto(clients[find_opponent(player)]->descriptor, buffer, MAX_MESSAGE_LENGTH, 0,
                   &clients[find_opponent(player)]->addr, sizeof(struct addrinfo));
        } else if (strcmp(command, "quit") == 0) {
            remove_client(name);
        } else if (strcmp(command, "alive") == 0) {
            int player = -1;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
                    player = i;
                }
            }
            if (player != -1) {
                clients[player]->online = 1;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}