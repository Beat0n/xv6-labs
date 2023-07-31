//! xargs.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int main(int argc, char* argv[]) {
    char buf[1024];
    char* pr = buf;
    char* pl = buf;
    char** args = (char**)malloc(MAXARG * sizeof(char*));
    int n = 0;

    if (argc < 2) {
        fprintf(2, "usage: xargs [program name] [params ...]\n");
        exit(1);
    }

    for (int i=1;i<argc;i++) {
        args[n] = argv[i];
        n++;
    }

    const int init_n = n;

    while (read(0, pr, 1)) {
        if (*pr == ' ' || *pr == '\n') {
            char tmp = *pr;
            *pr = '\0';
            args[n] = pl;
            n++;
            pl = pr+1;

            if(tmp == '\n') {
                args[n] = 0;
                if (fork() == 0) {
                    exec(argv[1], args);
                    exit(0);
                }
                for (int i=init_n;i<n;i++) {
                    args[i] = 0;
                }
                n = init_n;
            }
        }
        pr++;
    }

    if(n >= init_n) {
        *pr = '\0';
        args[n] = pl;
        n++;
        args[n] = 0;
        if (fork() == 0) {
            exec(argv[1], args);
            exit(0);
        }
    }
    while(wait(0) != -1) {};
    free(args);
    exit(0);
}