#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    char a[10] = "123456789\0";
    char *b = &a[3];
    printf("%s", b);
//    int fd[2];
//    if (pipe(fd) == -1) return 1;
//
//    int pid1 = fork();
//    if (pid1 < 0) return 2;
//    if (pid1 == 0) {
//        // Child process 1
//        dup2(fd[1], STDOUT_FILENO);
//        close(fd[0]);
//        close(fd[1]);
//        execlp("echo", "echo", "gfbfgbddg", NULL);
//    }
//
//    for (int i = 0; i < 3; i++) {
//        int fd_2[2];
//        pipe(fd_2);
//        pid_t pid2 = fork();
//        if (pid2 == 0) {
//            dup2(fd[0], STDIN_FILENO);
//            close(fd[0]);
//            close(fd[1]);
//            dup2(fd_2[1], STDOUT_FILENO);
//            close(fd_2[0]);
//            close(fd_2[1]);
//            execlp("cat", "cat", NULL);
//        } else {
//            close(fd[0]);
//            close(fd[1]);
//            fd[0] = fd_2[0];
//            fd[1] = fd_2[1];
//        }
//    }
//    int pid2 = fork();
//    if (pid2 < 0) return 3;
//    if (pid2 == 0) {
//        // Child process 2
//        dup2(fd[0], STDIN_FILENO);
//        close(fd[0]);
//        close(fd[1]);
//        execlp("wc", "wc", "-c", NULL);
//    }
//
//
//    close(fd[0]);
//    close(fd[1]);
//
//    waitpid(pid1, NULL, 0);
//    waitpid(pid2, NULL, 0);
//    return 0;
}