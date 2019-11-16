#include <vector>
#include <iostream>
#include <functional>
#include <assert.h>
#include <omp.h>
#include "DynamicLib.cc"
#include "util.hh"

const bool delay = true;

// This is a dumb hack to randomize targets/test2.so
// It uses the time for a seed,
// so I want to start both trials at random times,
// to verify that they get different results.
// Getting different results in both threads,
// indicates that the libraries really are loaded and initialized twice.
#include <chrono>
#include <thread>
#include <random>
void random_delay() {
	std::random_device rd;
	std::uniform_int_distribution<int> dist {0, 20};
	std::this_thread::sleep_for(std::chrono::milliseconds(dist(rd) * 15));
}


int run_main(const std::vector<std::string>& args) {
	DynamicLib lib (args[0]);

	int (*this_main)(int argc, char* argv[]);
	this_main = (int(*)(int, char**)) lib["main"];

    char** argv = new char*[args.size()];
	for (size_t i = 0; i < args.size(); ++i) {
		argv[i] = new char[args[i].size()];

		// this_main() could modify, so args[i].c_str won't work.
		std::copy(args[i].cbegin(), args[i].cend(), argv[i]);
	}

	int ret = this_main(args.size(), argv);

	for (size_t i = 0; i < args.size(); ++i) {
		delete[] argv[i];
	}
	delete[] argv;

	return ret;
}

int main(int argc, char* const argv[]) {
	std::string sep {";"};
	std::vector<std::string> args {argv, argv + argc};
	// args[0] was this program name
	args[0] = sep;
	// sep means "new sub arg list starts now"
	std::vector<std::vector<std::string>> sub_args;
	for (std::string arg : args) {
		if (arg.compare(sep) == 0) {
			sub_args.push_back(std::vector<std::string>{});
		} else {
			assert(sub_args.size() > 0);
			(sub_args.end() - 1)->push_back(std::move(arg));
		}
	}

	std::vector<int> results(sub_args.size(), 0);

	#pragma omp parallel for
	for (std::size_t i = 0; i < sub_args.size(); ++i) {
		if constexpr(delay) {
			random_delay();
		}
		results[i] = run_main(sub_args[i]);
	}

	for (std::size_t i = 0; i < sub_args.size(); ++i) {
		std::cout << sub_args[i][0] << ":main ";
		join(std::cout, sub_args[i].begin() + 1, sub_args[i].end(), "", " ", "");
		std::cout << std::endl << "returned " << results[i] << std::endl;
	}

	return 0;
}
