#include <iostream>

int main() {
	std::cout << "test1\n";
	// using \n instead of std::endl so that output is less likely to
	// get mangled together with other process-as-thread
	return 0;
}
