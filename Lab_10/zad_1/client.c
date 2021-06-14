#define MAX_MESSAGE_LENGTH 256

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef enum {
    FREE,
    O,
    X
} Field;

typedef enum {
    GAME_STARTING,
    WAITING,
    WAITING_FOR_MOVE,
    MOVE_OPPONENT,
    MOVE,
    QUIT
} State;

typedef struct {
    int move;
    Field objects[9];
} Board;

int move(Board *board, int position) {
    if (position < 0 || position > 9 || board->objects[position] != FREE)
        return 0;
    board->objects[position] = board->move ? O : X;
    board->move = !board->move;
    return 1;
}

Field check_winner(Board *board) {
    for (int x = 0; x < 3; x++) {
        // column
        if (board->objects[x] == board->objects[x + 3] && board->objects[x + 3] == board->objects[x + 6] &&
            board->objects[x] != FREE) {
            return board->objects[x];
        }
        // row
        if (board->objects[3 * x] == board->objects[3 * x + 1] &&
            board->objects[3 * x + 1] == board->objects[3 * x + 2] && board->objects[3 * x] != FREE) {
            return board->objects[3 * x];
        }
    }

    if (board->objects[0] == board->objects[4] && board->objects[4] == board->objects[8] && board->objects[0] != FREE) {
        return board->objects[0];
    }

    if (board->objects[2] == board->objects[4] && board->objects[4] == board->objects[6] && board->objects[2] != FREE) {
        return board->objects[2];
    }
    return FREE;
}

char *command, *arg;
int server_socket;
int is_client_O;
Field player_symbol;
char buffer[MAX_MESSAGE_LENGTH + 1];
char *name;

Board board;
State state = GAME_STARTING;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void quit() {
    char buffer[MAX_MESSAGE_LENGTH + 1];
    sprintf(buffer, "quit: :%s", name);
    send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    exit(0);
}

void check_game() {
    Field winner = check_winner(&board);
    if (winner != FREE) {
        if (player_symbol == winner) {
            printf("WIN!\n");
        } else {
            printf("LOST!\n");
        }
        state = QUIT;
        return;
    }
    for (int i = 0; i < 9; i++) {
        if (board.objects[i] == FREE) {
            printf("DRAW\n");
            state = QUIT;
            return;
        }
    }
}

void split(char *reply) {
    command = strtok(reply, ":");
    arg = strtok(NULL, ":");
}

Board new_board() {
    Board new_board = {};
    return new_board;
}

void draw() {
    char symbol;
    printf("^^:::::::::::::::::::^^\n");
    for (int y = 0; y < 3; y++) {
        printf("^^  ");
        for (int x = 0; x < 3; x++) {
            if (board.objects[y * 3 + x] == FREE) {
                symbol = y * 3 + x + 1 + '0';
            } else if (board.objects[y * 3 + x] == O) {
                symbol = 'O';
            } else {
                symbol = 'X';
            }
            printf("  %c  ", symbol);
        }
        printf("  ^^\n^^:::::::::::::::::::^^\n");
    }
}

void *play_game() {
    while (1) {
        if (state == GAME_STARTING) {
            if (strcmp(arg, "name_taken") == 0) {
                printf("Name is already taken\n");
                exit(1);
            } else if (strcmp(arg, "no_enemy") == 0) {
                printf("Waiting for opponent\n");
                state = WAITING;
            } else {
                board = new_board();
                player_symbol = (arg[0] == 'O') ? O : X;
                state = (player_symbol == O) ? MOVE : WAITING_FOR_MOVE;
            }
        } else if (state == WAITING) {
            pthread_mutex_lock(&mutex);
            while (state != GAME_STARTING && state != QUIT) {
                pthread_cond_wait(&cond, &mutex);
            }
            pthread_mutex_unlock(&mutex);

            board = new_board();
            player_symbol = (arg[0] == 'O') ? O : X;
            state = (player_symbol == O) ? MOVE : WAITING_FOR_MOVE;
        } else if (state == WAITING_FOR_MOVE) {
            printf("Waiting for opponent move\n");
            pthread_mutex_lock(&mutex);
            while (state != MOVE_OPPONENT && state != QUIT) {
                pthread_cond_wait(&cond, &mutex);
            }
            pthread_mutex_unlock(&mutex);
        } else if (state == MOVE_OPPONENT) {
            int pos = atoi(arg);
            move(&board, pos);
            check_game();
            if (state != QUIT) {
                state = MOVE;
            }
        } else if (state == MOVE) {
            draw();
            int pos;
            do {
                printf("Next move (%c): ", (player_symbol == O) ? 'O' : 'X');
                scanf("%d", &pos);
                pos--;
            } while (!move(&board, pos));

            draw();

            char buffer[MAX_MESSAGE_LENGTH + 1];
            sprintf(buffer, "move:%d:%s", pos, name);
            send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);

            check_game();
            if (state != QUIT) {
                state = WAITING_FOR_MOVE;
            } else {
                quit();
            }
        } else if (state == QUIT) {
            quit();
        }
    }
}

void init_server_connection(char *type, char *destination) {

    if (strcmp(type, "local") == 0) {
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un local_sockaddr;
        memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
        local_sockaddr.sun_family = AF_UNIX;
        strcpy(local_sockaddr.sun_path, destination);
        connect(server_socket, (struct sockaddr *) &local_sockaddr, sizeof(struct sockaddr_un));
    } else {
        struct addrinfo *info;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo("localhost", destination, &hints, &info);
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        connect(server_socket, info->ai_addr, info->ai_addrlen);
        freeaddrinfo(info);
    }
}

void listen_server() {
    int game_thread_running = 0;
    while (1) {
        recv(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
        split(buffer);
        pthread_mutex_lock(&mutex);
        if (strcmp(command, "add") == 0) {
            state = GAME_STARTING;
            if (!game_thread_running) {
                pthread_t t;
                pthread_create(&t, NULL, play_game, NULL);
                game_thread_running = 1;
            }
        } else if (strcmp(command, "move") == 0) {
            state = MOVE_OPPONENT;
        } else if (strcmp(command, "quit") == 0) {
            state = QUIT;
            exit(0);
        } else if (strcmp(command, "ping_thread_function") == 0) {
            sprintf(buffer, "alive: :%s", name);
            send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "./client [name] [type] [destination]");
        return 1;
    }

    name = argv[1];
    char *type = argv[2];
    char *destination = argv[3];

    signal(SIGINT, quit);
    init_server_connection(type, destination);
    char buffer[MAX_MESSAGE_LENGTH + 1];
    sprintf(buffer, "add: :%s", name);
    send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);

    listen_server();
    return 0;
}