#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include "execves.h"

int main(int argc, char **argv) {
    execves_attr_t attrs;
    char** args = {NULL};
    char** envp = {NULL};
    long res = syscall(__NR_execves, "./test.sh", args, envp, &attrs);
    return res;
}
