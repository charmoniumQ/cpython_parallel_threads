#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define __NR_execves 436

typedef struct {
    int unused;
} execves_attr_t;

int main(int argc, char **argv) {
    execves_attr_t attrs;
    long res = syscall(__NR_execves, "./test.sh", {NULL}, {NULL}, &attrs);
    return res;
}
