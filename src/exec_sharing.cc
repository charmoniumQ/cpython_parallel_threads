#include <vector>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <execution>

#include "dynamic_lib.hh"
#include "util.hh"

int run_main(const std::vector<std::string>& args) {
	dynamic_libs lib = dynamic_libs::create({
		// {"/lib64/ld-linux-x86-64.so.2", {}},
		// {"/lib/x86_64-linux-gnu/libc.so.6", {}},
		// {"/lib/x86_64-linux-gnu/libm.so.6", {}},
		// {"/lib/x86_64-linux-gnu/libgcc_s.so.1", {}},
		// {"/usr/lib/x86_64-linux-gnu/libstdc++.so.6", {}},
		{"/lib/x86_64-linux-gnu/libdl.so.2", {}},
		{args[0], {"main"}},
	});

	typedef int(*main_method)(int, char**);
	main_method this_main = lib.get<main_method>("main");

	char** argv = strings2char_pptr(args);
	int ret = this_main(args.size(), argv);
	free_char_pptr(args.size(), argv);

	return ret;
}

// shamelessly hoisted from
// https://stackoverflow.com/a/53001490/1078199
enum class Policy {seq, unseq, par_seq, par_unseq};
template<class Iterator, class F>
auto maybe_parallel_for_each(Policy policy, Iterator begin, Iterator end, F f) {
	switch(policy) {
	case Policy::par_unseq: return std::for_each(std::execution::par_unseq, begin, end, f);
	case Policy::    unseq: return std::for_each(std::execution::    unseq, begin, end, f);
	case Policy::      seq: return std::for_each(std::execution::      seq, begin, end, f);
	case Policy::  par_seq: return std::for_each(std::execution::par      , begin, end, f);
	}
}

int main(int argc, char* const argv[]) {
	std::string sep {";"};
	std::vector<std::string> args {argv, argv + argc};

	// parse flag arguments
	Policy policy = Policy::par_unseq;
	if (false) {}
	else if (args[1] == "-par_unseq") {
		policy = Policy::par_unseq;
		args.erase(args.begin() + 1);
	}
	else if (args[1] == "-unseq") {
		policy = Policy::unseq;
		args.erase(args.begin() + 1);
	}
	else if (args[1] == "-par_seq") {
		policy = Policy::par_seq;
		args.erase(args.begin() + 1);
	}
	else if (args[1] == "-seq") {
		policy = Policy::seq;
		args.erase(args.begin() + 1);
	}
	// no more flag arguments present in args

	// args[0] was this program name
	args[0] = sep;
	// sep means "new sub arg list starts now"
	std::vector<std::vector<std::string>> sub_args;
	for (std::string arg : args) {
		if (arg == sep) {
			sub_args.push_back(std::vector<std::string>{});
		} else {
			assert(sub_args.size() > 0);
			// steal/move the arg
			// I promise not to access "args" again
			(sub_args.end() - 1)->push_back(std::move(arg));
		}
	}

	maybe_parallel_for_each(
		policy,
		sub_args.begin(),
		sub_args.end(),
		[](auto&& sub_arg) {

			int ret = 0;
			try {
				ret = run_main(sub_arg);
			} catch (const std::runtime_error& e) {
				std::cout << "load ";
				join(std::cout, sub_arg, "\"", "\", \"", "\" ");
				std::cout << "failed: " << e.what() << std::endl;
			}

			if (ret != 0) {
				join(std::cout, sub_arg, "\"", "\", \"", "\" ");
				std::cout << "returned " << ret << std::endl;
			}
		}
	);

	return 0;
}
