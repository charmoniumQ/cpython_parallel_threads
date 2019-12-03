#include <cstring>
#include <cstdlib>
#include <iostream>

#include "DynamicLib.cc"
#include "util.hh"

int main(int argc, char** argv) {
	std::string python_so = "/usr/lib/x86_64-linux-gnu/libpython3.7m.so";
	// TODO: get this value programatically by
	// $ sbin/ldconfig -p | grep -o '\S*libpython3.7m.so$'
	// overridable by env var
	DynamicLib lib {quick_tmp_copy(python_so)};

	auto Py_Main = lib.get
		<int (*)(int, wchar_t**)>
		("Py_Main");

	wchar_t** argw = new wchar_t*[argc];
	for (int i = 0; i < argc; ++i) {
		size_t length = strlen(argv[i]) * 4;
		argw[i] = new wchar_t[length];
		mbstowcs(argw[i], argv[i], length);
	}

	int ret = Py_Main(argc, argw);

	for (int i = 0; i < argc; ++i) {
		delete[] argw[i];
	}
	delete[] argw;

	return ret;
}
