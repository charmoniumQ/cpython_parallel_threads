#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include "execves.h"

int main(int argc, char **argv) {
    long res;
    execves_attr_t attrs;
    char path[] = "./test.sh";
    char* args[] = {path, "Hello", "World", NULL};
    char* envp[] = {"abc=123", NULL};

    res = syscall(__NR_execves, path, args, envp, &attrs);
    printf("execves returned %ld\n", res);
    return 0;
}
