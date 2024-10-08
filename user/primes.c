#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define END_NUMBER 35

void prime_sieve(int* nums, int count) {
  int pipe_fd[2];
  if(pipe(pipe_fd) == -1) {
    fprintf(2, "create pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0) {
    fprintf(2, "create child process failed\n");
    exit(1);
  }
  if(pid == 0) { // child process
    close(pipe_fd[1]);

    char buffer[4] = {0};
    read(pipe_fd[0], buffer, 4);
    int first = *(int*)(buffer);
    printf("prime %d\n", first);
    int new_count = 0;

    for(int i=1; i<count; ++i) {
      read(pipe_fd[0], buffer, 4);
      int num = *(int*)(buffer);
      if(num % first != 0) {
        *(nums + new_count) = num;
        new_count++;
      }
    }
    if(new_count > 0) prime_sieve(nums, new_count);
  } else {
    close(pipe_fd[0]);

    for(int i=0; i<count; ++i) {
      write(pipe_fd[1], nums + i, 4);
    }
    close(pipe_fd[1]);
    wait(0);
  }

}

int
main(int argc, char *argv[])
{
  int nums[END_NUMBER-1];
  for(int i=0; i<END_NUMBER-1; ++i) {
    nums[i] = i + 2;
  }
  prime_sieve(nums, END_NUMBER-1);
  exit(0);
}