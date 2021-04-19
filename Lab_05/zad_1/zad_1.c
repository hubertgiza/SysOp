#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define LINE_LENGTH 100
#define NUMBER_OF_ARGUMENTS 10
#define NUMBER_OF_COMPONENTS 100

struct block {
    char **array;
};

struct pipe {
    struct block *blocks;
    char *name;
};

char **split_command_to_words(char *command) {
    if (command == NULL) {
        return NULL;
    }
    char **result = calloc(NUMBER_OF_ARGUMENTS, sizeof(char *));
    int last_index = 0;
    int arguments = 0;
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == ' ' || i == strlen(command) - 1) {
            result[arguments] = calloc(i - last_index, sizeof(char));
            int k = 0;
            for (int j = last_index; j < i; j++) {
                if (command[last_index + k] != ' ') {
                    result[arguments][k] = command[last_index + k];
                    k++;
                } else {
                    result[arguments][k] = '\0';
                    k++;
                }

            }
            if ((i == strlen(command) - 1) && (command[i] != ' ') && (command[i] != '\n')) {
                result[arguments][k] = command[i];
            }
            arguments++;
            last_index = i + 1;
        }
    }
    return result;
}

char **split_component_into_commands(char *component) {
    if (component == NULL) {
        return NULL;
    }
    char **result = calloc(NUMBER_OF_ARGUMENTS, sizeof(char *));
    int old_index = 0;
    int commands = 0;
    for (int i = 0; i < strlen(component); i++) {
        if (component[i] == '|' || i == strlen(component) - 1) {
            result[commands] = calloc(i - old_index, sizeof(char));
            for (int j = old_index; j < i; j++) {
                result[commands][j - old_index] = component[j];
            }
            if ((i == strlen(component) - 1) && (component[i] != '\n') && (component[i] != ' ')) {
                result[commands][i - old_index] = component[i];
            }
            old_index = i + 2;
            commands++;
        }
    }
    return result;
}

int check_if_component(char *component) {
    for (int i = 0; i < strlen(component); i++) {
        if (component[i] == '=') {
            return 1;
        }
    }
    return 0;
}

struct pipe *parse_component(char *component) {
    struct pipe *res = calloc(1, sizeof(struct pipe));
    for (int i = 0; i < strlen(component); i++) {
        if (component[i] == ' ') {
            char *buff = calloc(i, sizeof(char));
            for (int j = 0; j < i; j++) {
                buff[j] = component[j];
            }
            res->name = buff;
            res->blocks = calloc(NUMBER_OF_ARGUMENTS, sizeof(struct block));
            char **commands = split_component_into_commands(&component[i + 3]);
            for (int j = 0; j < NUMBER_OF_ARGUMENTS; j++) {
                if (commands[j] != NULL) {
                    res->blocks[j].array = split_command_to_words(commands[j]);
                }
            }
            break;
        }
    }
    return res;
}

void my_execlp(struct block command) {
    execlp(command.array[0], command.array[0], command.array[1], command.array[2], command.array[3],
           command.array[4], command.array[5], command.array[6], command.array[7], command.array[8],
           command.array[9], NULL);
}

int get_number_of_commands(struct pipe *x) {
    if (x == NULL) {
        return 0;
    }
    int res = 0;
    for (int i = 0; i < NUMBER_OF_ARGUMENTS; i++) {
        if (x->blocks[i].array != NULL) {
            if (x->blocks[i].array[0] != NULL) {
                res++;
            }
        } else {
            return res;
        }
    }
    return res;
}

char **get_list_of_components_to_pipe(char *pipe) {
    if (pipe == NULL) {
        return NULL;
    }
    char **result = calloc(NUMBER_OF_ARGUMENTS, sizeof(char *));
    int old_index = 0;
    int commands = 0;
    for (int i = 0; i < strlen(pipe); i++) {
        if (pipe[i] == '|' || i == strlen(pipe) - 1) {
            result[commands] = calloc(i - old_index, sizeof(char));
            for (int j = old_index; j < i; j++) {
                if (pipe[j] != ' ') {
                    result[commands][j - old_index] = pipe[j];
                }
            }
            if ((i == strlen(pipe) - 1) && (pipe[i] != ' ' && pipe[i] != '\n')) {
                result[commands][i - old_index] = pipe[i];
            }
            old_index = i + 2;
            commands++;
        }
    }
    return result;

}

void exec_pipeline(struct pipe *x) {
    int commands = get_number_of_commands(x);

    int fd[2];
    if (pipe(fd) == -1) exit(2);

    int pid1 = fork();
    if (pid1 < 0) exit(2);
    if (pid1 == 0) {
        // Child process 1
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        my_execlp(x->blocks[0]);
    }

    for (int i = 1; i < commands - 1; i++) {
        int fd_2[2];
        pipe(fd_2);
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);
            dup2(fd_2[1], STDOUT_FILENO);
            close(fd_2[0]);
            close(fd_2[1]);
            my_execlp(x->blocks[i]);
        } else {
            close(fd[0]);
            close(fd[1]);
            fd[0] = fd_2[0];
            fd[1] = fd_2[1];
        }
    }
    int pid2 = fork();
    if (pid2 < 0) exit(2);
    if (pid2 == 0) {
        // Child process 2
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        my_execlp(x->blocks[commands - 1]);
    }


    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        printf("Provided %d, when expected 1", argc - 1);
        exit(0);
    }
    char *file_name = argv[1];
    char *component = calloc(LINE_LENGTH, sizeof(char));
    FILE *file = fopen(file_name, "r");
    struct pipe *all_components = calloc(NUMBER_OF_COMPONENTS, sizeof(struct pipe));
    struct pipe *all_streams = calloc(NUMBER_OF_COMPONENTS, sizeof(struct pipe));
    int n_components = 0;
    int n_streams = 0;
    while (fgets(component, LINE_LENGTH, file) != NULL) {
        if (check_if_component(component)) {
            all_components[n_components] = *parse_component(component);
            n_components++;
        } else if (strcmp(component, "\n") != 0) {
            char **pipeline_components = get_list_of_components_to_pipe(component);
//            for (int i = 0; i < NUMBER_OF_ARGUMENTS; i++) {
//                if (pipeline_components[i] != NULL) {
//                    printf("%s.\n", pipeline_components[i]);
//                }
//            }
            int length_of_stream = 0;
            for (int i = 0; i < NUMBER_OF_ARGUMENTS; i++) {
                for (int j = 0; j < n_components; j++) {
                    if ((all_components[j].name != NULL) && (pipeline_components[i] != NULL)) {
                        if (strcmp(all_components[j].name, pipeline_components[i]) == 0) {
                            length_of_stream += get_number_of_commands(&all_components[j]);
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
            all_streams[n_streams].blocks = calloc(length_of_stream + 1, sizeof(struct block));
            int l = 0;
            for (int i = 0; i < NUMBER_OF_ARGUMENTS; i++) {
                for (int j = 0; j < n_components; j++) {
                    if ((all_components[j].name != NULL) && (pipeline_components[i] != NULL)) {
                        if (strcmp(all_components[j].name, pipeline_components[i]) == 0) {
                            for (int k = 0; k < get_number_of_commands(&all_components[j]); k++) {
                                all_streams[n_streams].blocks[l] = all_components[j].blocks[k];
                                l++;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
            all_streams[n_streams].blocks[length_of_stream].array = NULL;
            for (int i = 0; i < length_of_stream; i++) {
                printf("%s\n", all_streams[n_streams].blocks[i].array[0]);
            }
            n_streams++;
        }
    }
//    exec_pipeline(&all_streams[0]);
    for (int i = 0; i < n_streams; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            exec_pipeline(&all_streams[i]);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
    wait(NULL);
    return 0;
}