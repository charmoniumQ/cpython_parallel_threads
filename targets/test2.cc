#include <unistd.h>
#include <iostream>
#include <thread>
#include <random>
#include <sstream>

int main() {
	std::random_device rd;
	std::uniform_int_distribution<int> dist {0, 20};

	std::ostringstream ss;
	ss << "pid " << getpid() << " ";
	ss << "tid " << std::this_thread::get_id() << " ";
	ss << "rand " << dist(rd) << " ";
	std::cout << ss.str() << std::endl;
	return 0;
}
