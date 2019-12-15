#include <cassert>
#include <iostream>
#include <random>
#include <sstream>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

namespace {
	struct initializer {
		initializer() {
			// using \n instead of std::endl makes it less likely that the
			// newline gets mangled with the output of other threads.
			std::cout << "ctor\n";
		}

		~initializer() {
			std::cout << "dtor\n";
		}
	};
	static initializer i;
}

int main() {
	std::default_random_engine rd {0};
	std::uniform_int_distribution<int> dist {0, 20};

	std::ostringstream ss;
	ss << "pid " << getpid() << "\n";

	// this doesn't work
	// ss << "tid " << std::this_thread::get_id() << "\n";

#ifdef SYS_gettid
	// because gettid
	pid_t tid = syscall(SYS_gettid);
#else
#error "SYS_gettid unavailable on this system"
#endif
	ss << "tid " << tid << "\n";
	ss << "rand " << dist(rd) << "\n";
	std::cout << ss.str();
	return 0;
}
