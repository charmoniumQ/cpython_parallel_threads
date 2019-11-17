#include <unistd.h>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>

static void ctor(void) __attribute__((constructor));
static void ctor(void) {
	std::cout << "ctor\n";
}
static void dtor(void) __attribute__((destructor));
static void dtor(void) {
	std::cout << "dtor\n";
}

int main() {
	std::default_random_engine rd {0};
	std::uniform_int_distribution<int> dist {0, 20};

	std::ostringstream ss;
	ss << "pid " << getpid() << "\n";
	ss << "tid " << std::this_thread::get_id() << "\n";
	ss << "rand " << dist(rd) << "\n";
	std::cout << ss.str();
	return 0;
}
