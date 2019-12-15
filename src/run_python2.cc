#include <cstring>
#include <cstdlib>
#include <iostream>

#include "dynamic_lib.hh"
#include "util.hh"
#include "python_so_path.cc"

int main(int argc, char** argv) {
	dynamic_libs lib = dynamic_libs::create({
		{get_python_so(), {"Py_Main"}},
	});

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
