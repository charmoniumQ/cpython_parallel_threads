#include <vector>
#include <iostream>
#include <functional>
#include <omp.h>
#include "DynamicLib.cc"

#include <time.h>
#include <unistd.h>
void rand_delay() {
	usleep(rand() % 1000000);
}

typedef std::function<int(int, const char**)> main_t;

int run_main(std::string path) {
	DynamicLib lib (path);

	int (*this_main)(int, const char**);
	this_main = (int(*)(int, const char**)) lib["main"];

	const char* this_argv[] = {path.c_str(), NULL};

	rand_delay();
	return this_main(1, this_argv);
}

int main(int argc, char* const argv []) {
	std::vector<std::string> libs(argv + 1, argv + argc);
	std::vector<int> results(argc - 1, 0);

	#pragma omp parallel for
	for (std::size_t i = 0; i < libs.size(); ++i) {
		results[i] = run_main(libs[i]);
	}

	for (std::size_t i = 0; i < libs.size(); ++i) {
		std::cout << libs[i] << ":main() returned " << results[i] << std::endl;
	}
	return 0;
}
