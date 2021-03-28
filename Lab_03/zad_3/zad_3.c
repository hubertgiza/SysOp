#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>

#define LINE_LENGTH 256
#define PATH_LENGTH 512

int check_how_many_directories(char *directory) {
    int number_of_directories = 0;
    struct dirent *dir;
    DIR *d = opendir(directory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == 4) {
                if (strcoll(dir->d_name, ".") && strcoll(dir->d_name, "..")) {
                    number_of_directories++;
                }
            }
        }
        closedir(d);
    }
    return number_of_directories;
}

char **get_directories_names(int number_of_directories, char *directory) {
    int i = 0;
    char **results = calloc(number_of_directories, sizeof(char *));
    struct dirent *dir;
    DIR *d = opendir(directory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == 4) {
                if (strcoll(dir->d_name, ".") && strcoll(dir->d_name, "..")) {
                    int len = strlen(dir->d_name);
                    char *name = calloc(len, sizeof(char));
                    strcpy(name, dir->d_name);
                    results[i] = name;
                    i++;
                }
            }
        }
        closedir(d);
    }
    return results;
}

int check_if_sequence_in_line(char *line, char *sequence) {
    int j = 0;
    int len_sequence = strlen(sequence);
    for (int i = 0; i < LINE_LENGTH; i++) {
        if (line[i] != '\n' && line[i] != EOF) {
            if (line[i] == sequence[j]) {
                j++;
                if (j == len_sequence) {
                    return 1;
                }
            } else {
                j = 0;
            }
        }
    }
    return 0;
}

int check_if_sequence_in_file(char *filename, char *sequence) {
    FILE *file = fopen(filename, "r");
    char *line = calloc(LINE_LENGTH, sizeof(char));
    while (fgets(line, LINE_LENGTH, file) != NULL) {
        if (check_if_sequence_in_line(line, sequence)) {
            return 1;
        }
    }
    return 0;
}

int get_number_of_found_files_in_directory(char *directory, char *sequence) {
    int n = 0;
    struct dirent *dir;
    DIR *d = opendir(directory);
    regex_t regex;
    int val;
    val = regcomp(&regex, "[.]txt$", 0);
    if (val) {
        printf("Could not compile regex\n");
        exit(1);
    }
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == 8) {
                val = regexec(&regex, dir->d_name, 0, NULL, 0);
                if (val == 0) {
                    if (check_if_sequence_in_file(dir->d_name, sequence)) {
                        n++;
                    }
                }
            }
        }
    }
    closedir(d);
    return n;
}

char **print_found_files_in_directory(char *directory, char *sequence, char *actual_path, int pid) {
    int n = get_number_of_found_files_in_directory(directory, sequence);
    char **files = calloc(n, sizeof(char *));
    int i = 0;
    struct dirent *dir;
    DIR *d = opendir(directory);
    regex_t regex;
    int val;
    val = regcomp(&regex, "[.]txt$", 0);
    if (val) {
        printf("Could not compile regex\n");
        exit(1);
    }
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == 8) {
                val = regexec(&regex, dir->d_name, 0, NULL, 0);
                if (val == 0) {
                    if (check_if_sequence_in_file(dir->d_name, sequence)) {
                        files[i] = calloc(LINE_LENGTH, sizeof(char));
                        files[i] = dir->d_name;
                        i++;
                    }
                }
            }
        }
    }
    closedir(d);
    return files;
}

void recur_fork_new_folder(int current_depth, int max_depth, char *curr_dir, char *sequence, char *actual_path) {
    int n = check_how_many_directories(curr_dir);
    char **directories_to_search = get_directories_names(n, curr_dir);
    if (current_depth >= max_depth) {
        return;
    } else if (n == 0) {
        return;
    } else {
        for (int i = 0; i < n; i++) {
            if (fork() == 0) {
                printf("wchodzi");
//                char **files = print_found_files_in_directory(directories_to_search[i], sequence, actual_path,
//                                                              getppid());
//                int j = 0;
//                while (files[j] != NULL) {
//                    printf("PID: %d     PATH:%s/%s", getppid(), actual_path, files[j]);
//                    j++;
//                }
                exit(0);
            } else {
                free(actual_path);
                actual_path = calloc(PATH_LENGTH, sizeof(char));
                getcwd(actual_path, PATH_LENGTH);
//                recur_fork_new_folder(current_depth + 1, max_depth, directories_to_search[i], sequence, actual_path);
            }
        }
    }

}

int main(int argc, char **argv) {
    if (argc != 4) {
        perror("Wrong number of arguments!\n");
        printf("Provided %d arguments, when required 3\n", argc - 1);
        exit(-1);
    }
    DIR *d;
    struct dirent *dir;
    char *directory = argv[1];
    char *content = argv[2];
    char *curr_dir = calloc(LINE_LENGTH, sizeof(char));
    curr_dir[0] = '.';
    curr_dir[1] = '\0';
    char *path = calloc(PATH_LENGTH, sizeof(char));
    path[0] = '.';
    path[1] = '\0';
    unsigned int depth = strtol(argv[3], NULL, 10);
    //TODO na while zmienic zamiast rekurencji
    recur_fork_new_folder(0, depth, curr_dir, content, path);
}