#include <stdio.h>

int main(int argc, char **argv) {
    for (unsigned int i = 0; i < argc; ++i) {
	printf("%d: %s\n", i, argv[i]);
    }
    return 0;
}
