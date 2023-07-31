//! primes.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void run(int pipe_left[2]) {
    int min_prime;
    if (!read(pipe_left[0], &min_prime, sizeof(min_prime))) exit(0);
    printf("prime: %d\n", min_prime);

    int pipe_right[2];
    pipe(pipe_right);
    if (fork() == 0) {
        close(pipe_right[1]);
        run(pipe_right);
        exit(0);
    } else {
        close(pipe_right[0]);
        int buf;
        while (read(pipe_left[0], &buf, sizeof(buf))) {
            if ((buf % min_prime) != 0) {
                write(pipe_right[1], &buf, sizeof(buf));
            }
        }
        close(pipe_right[1]);
    }
    wait(0);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(2, "usage: primes [num]");
        exit(1);
    }
    int _pipe[2];
    pipe(_pipe);

    if (fork() == 0) {
        close(_pipe[1]);
        run(_pipe);
        exit(0);
    } else {
        close(_pipe[0]);
        int i;
        for (i = 2; i<=atoi(argv[1]); i++) {
            write(_pipe[1], &i, sizeof(i));
        }
        close(_pipe[1]);
    }
    wait(0);
    exit(0);
}