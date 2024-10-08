#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 10

int
main(int argc, char *argv[])
{
  int pipe_fd1[2];
  int pipe_fd2[2];
  if(pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1) {
    fprintf(2, "create pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0) {
    fprintf(2, "create child process failed\n");
    exit(1);
  }
  if(pid == 0) { // child process
    close(pipe_fd1[0]);
    close(pipe_fd2[1]);

    char buffer[BUFFER_SIZE] = {0};
    read(pipe_fd2[0], buffer, BUFFER_SIZE);
    printf("%d: received %s\n", getpid(), buffer);
    write(pipe_fd1[1], "pong", 4);
  } else {
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);

    write(pipe_fd2[1], "ping", 4);
    char buffer[BUFFER_SIZE] = {0};
    read(pipe_fd1[0], buffer, BUFFER_SIZE);
    printf("%d: received %s\n", getpid(), buffer);
  }
  exit(0);
}