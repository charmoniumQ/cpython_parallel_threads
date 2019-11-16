#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int a;

int main() {
	srand(clock()); 
	a = rand() % 1000;

	usleep(10000);
	// at this point, both writes should have gone through

	printf("%d\n", a);
	return 0;
}
