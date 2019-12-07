#include <cstring>
#include <cstdlib>
#include <iostream>

#include "dynamic_lib.hh"
#include "util.hh"

int main(int argc, char** argv) {
	std::string python_so = "/usr/lib/x86_64-linux-gnu/libpython3.7m.so";
	// TODO: get this value programatically by
	// $ sbin/ldconfig -p | grep -o '\S*libpython3.7m.so$'
	// overridable by env var
	dynamic_libs lib = dynamic_libs::create({
		{python_so, {"Py_Main"}},
	});

	// auto Py_Main = lib.get
	// 	<int (*)(int, wchar_t**)>
	// 	("Py_Main");
	// ((void)Py_Main);

	wchar_t** argw = new wchar_t*[argc];
	for (int i = 0; i < argc; ++i) {
		size_t length = strlen(argv[i]) * 4;
		argw[i] = new wchar_t[length];
		mbstowcs(argw[i], argv[i], length);
	}

	std::cout << "running "
			  // << ((void*)Py_Main)
			  << std::endl;
	// int ret = Py_Main(argc, argw);
	int ret =0 ;
	std::cout << "ran\n";

	for (int i = 0; i < argc; ++i) {
		delete[] argw[i];
	}
	delete[] argw;

	return ret;
}
