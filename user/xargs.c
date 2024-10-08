#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
  if(argc < 3) {
    fprintf(2, "usage: xargs {cmd} {args ...}");
    exit(1);
  } else {
    char *sub_argv[MAXARG];
    for(int i=1; i<argc; ++i) {
      sub_argv[i-1] = argv[i];
    }
    char buf[1024] = {0};
    int len = 0;
    while(read(0, buf+len, 1) > 0) {
      if(buf[len] == '\n' || buf[len] == ' ') {
        buf[len] = 0;
        sub_argv[argc-1] = buf;
        len = 0;
        sub_argv[argc] = 0;
        if(fork() == 0) {
          exec(argv[1], sub_argv);
        } else {
          wait(0);
        }
      } else {
        ++len;
      }
    }
    exit(0);
  }
}