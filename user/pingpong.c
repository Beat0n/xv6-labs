//! pingpong.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int pipe_1[2];
    int pipe_2[2];
    pipe(pipe_1);
    pipe(pipe_2);
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "fork failed!");
    } else if (pid == 0) {
        char buf;
        read(pipe_1[0], &buf, 1);
        printf("%d: received ping\n", getpid());
        write(pipe_2[1], &buf, 1);
    } else {
        char buf;
        write(pipe_1[1], "h", 1);
        read(pipe_2[0], &buf, 1);
        printf("%d: received pong\n", getpid());
        wait(0);
    }
    exit(0);
}