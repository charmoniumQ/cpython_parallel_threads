#include <unistd.h>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>

static void ctor(void) __attribute__((constructor));
static void ctor(void) {
	std::cout << "ctor" << std::endl;
}
static void dtor(void) __attribute__((destructor));
static void dtor(void) {
	std::cout << "dtor" << std::endl;
}

int main() {
	std::random_device rd;
	std::uniform_int_distribution<int> dist {0, 20};

	std::ostringstream ss;
	ss << "pid " << getpid() << "\n";
	ss << "tid " << std::this_thread::get_id() << "\n";
	ss << "rand " << dist(rd) << "\n";
	std::cout << ss.str() << std::endl;
	return 0;
}
