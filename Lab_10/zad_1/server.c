#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <poll.h>

#define MAX_PLAYERS 20
#define MAX_MESSAGE_LENGTH 256
#define MAX_BACKLOG 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
    char *name;
    int descriptor;
    int online;
} client;

client *clients[MAX_PLAYERS] = {NULL};
int clients_count = 0;

int register_client(char *name, int fd) {
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
        printf("Removing opponent: %s\n", clients[rival]->name);
        free(clients[rival]->name);
        free(clients[rival]);
        clients[rival] = NULL;
        clients_count--;
    }
}

void *ping_thread_function() {
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
                send(clients[i]->descriptor, "ping_thread_function: ", MAX_MESSAGE_LENGTH, 0);
                clients[i]->online = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(3);
    }
    return NULL;
}

int init_local_socket(char *path) {
    int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un local_sockaddr;
    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, path);
    unlink(path);
    int err = bind(local_socket, (struct sockaddr *) &local_sockaddr, sizeof(struct sockaddr_un));
    if (err == -1) {
        printf("%s\n", strerror(errno));
        exit(0);
    }
    listen(local_socket, MAX_BACKLOG);
    return local_socket;
}

int init_network_socket(char *port) {
    struct addrinfo *info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(NULL, port, &hints, &info);
    if (err == -1) {
        printf("%s\n", strerror(errno));
        exit(0);
    }
    int network_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    err = bind(network_socket, info->ai_addr, info->ai_addrlen);
    if (err == -1) {
        printf("%s\n", strerror(errno));
        exit(0);
    }
    listen(network_socket, MAX_BACKLOG);
    freeaddrinfo(info);

    return network_socket;
}

int check_messages(int local_socket, int network_socket) {
    struct pollfd *fds = calloc(2 + clients_count, sizeof(struct pollfd));
    fds[0].fd = local_socket;
    fds[0].events = POLLIN;
    fds[1].fd = network_socket;
    fds[1].events = POLLIN;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clients_count; i++) {
        fds[i + 2].fd = clients[i]->descriptor;
        fds[i + 2].events = POLLIN;
    }
    pthread_mutex_unlock(&mutex);
    poll(fds, clients_count + 2, -1);
    int retval;
    for (int i = 0; i < clients_count + 2; i++) {
        if (fds[i].revents & POLLIN) {
            retval = fds[i].fd;
            break;
        }
    }
    if (retval == local_socket || retval == network_socket) {
        retval = accept(retval, NULL, NULL);
    }
    free(fds);
    return retval;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Wrong number of arguments. Got %d, expected 2\n", argc - 1);
    }
    char *port = argv[1];
    char *socket_path = argv[2];
    srand(time(NULL));
    int local_socket = init_local_socket(socket_path);
    int network_socket = init_network_socket(port);
    pthread_t t;
    pthread_create(&t, NULL, ping_thread_function, NULL);
    while (1) {
        int client_fd = check_messages(local_socket, network_socket);
        char buffer[MAX_MESSAGE_LENGTH + 1];
        recv(client_fd, buffer, MAX_MESSAGE_LENGTH, 0);
        printf("%s\n", buffer);
        char *command = strtok(buffer, ":");
        char *arg = strtok(NULL, ":");
        char *name = strtok(NULL, ":");
        pthread_mutex_lock(&mutex);
        if (strcmp(command, "add") == 0) {
            int index = register_client(name, client_fd);
            if (index == -1) {
                send(client_fd, "add:name_taken", MAX_MESSAGE_LENGTH, 0);
                close(client_fd);
            } else if (index % 2 == 0) {
                send(client_fd, "add:no_enemy", MAX_MESSAGE_LENGTH, 0);
            } else {
                int random_num = rand() % 2;
                int player_1, player_2;
                if (random_num % 2 == 0) {
                    player_1 = index;
                    player_2 = find_opponent(index);
                } else {
                    player_2 = index;
                    player_1 = find_opponent(index);
                }

                send(clients[player_1]->descriptor, "add:O",
                     MAX_MESSAGE_LENGTH, 0);
                send(clients[player_2]->descriptor, "add:X",
                     MAX_MESSAGE_LENGTH, 0);
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
            send(clients[find_opponent(player)]->descriptor, buffer, MAX_MESSAGE_LENGTH, 0);
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

}