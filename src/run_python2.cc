#include <cstring>
#include <cstdlib>
#include <iostream>

#include "dynamic_lib.hh"
#include "util.hh"
#include "python_so_path.hh"

int main(int argc, char** argv) {
	int ret = 0;
	dynamic_libs lib = dynamic_libs::create({
		{get_python_so(), {
			"Py_BytesMain",
			"Py_InitializeEx",
			"Py_FinalizeEx",
		}},
	});

	auto Py_InitializeEx = lib.get
		<void(*)(int)>
		("Py_InitializeEx");
	auto Py_FinalizeEx = lib.get
		<int(*)()>
		("Py_FinalizeEx");
	auto Py_BytesMain = lib.get
		<int(*)(int, char**)>
		("Py_BytesMain");

	Py_InitializeEx(0);
	ret |= Py_BytesMain(argc, argv);
	ret |= Py_FinalizeEx();

	return ret;
}
