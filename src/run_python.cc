#include <deque>
#include <iostream>
#include <cstdio>
#include <cstring>

#include "DynamicLib.cc"
#include "util.hh"

template <typename T>
T get_and_pop_front(std::deque<T>& it) {
	T r = std::move(it[0]);
	it.pop_front();
	return r;
}

int main(int argc, char** argv) {
	int ret = 0;

	std::deque<std::string> args {argv, argv + argc};

	std::string prog_name = get_and_pop_front(args);

	std::string python_so = "/usr/lib/x86_64-linux-gnu/libpython3.7m.so";
	// TODO: get this value programatically by
	// $ sbin/ldconfig -p | grep -o '\S*libpython3.7m.so$'
	// overridable by env var
	DynamicLib lib {quick_tmp_copy(python_so)};

	auto Py_DecodeLocale = lib.get
		<wchar_t* (*)(const char*, size_t *)>
		("Py_DecodeLocale");
	auto Py_SetProgramName = lib.get
		<void (*)(const wchar_t *)>
		("Py_SetProgramName");
	auto Py_Initialize = lib.get
		<void (*)(void)>
		("Py_Initialize");
	auto PyRun_SimpleFile = lib.get
		<int (*)(FILE *, const char *)>
		("PyRun_SimpleFile");
	auto Py_FinalizeEx = lib.get
		<int (*)(void)>
		("Py_FinalizeEx");
	auto PyMem_RawFree = lib.get
		<void (*)(void*)>
		("PyMem_RawFree");
	auto PyRun_SimpleString = lib.get
		<int (*)(const char*)>
		("PyMem_RawFree");

	std::string path = get_and_pop_front(args);

	wchar_t* const program = Py_DecodeLocale(prog_name.c_str(), NULL);
	if (program == NULL) {
		std::cerr << "OOM" << std::endl;
		ret = 1;
	} else {

		Py_SetProgramName(program);
		Py_Initialize();

		if (path == "-c") {

			std::string prog = get_and_pop_front(args);
			char* prog_c_str = new char[prog.size() + 1];
			strcpy(prog_c_str, prog.c_str());
			std::cout << "Running: " << prog_c_str << std::endl;
			ret = PyRun_SimpleString(prog_c_str);
			if (ret != 0) {
				std::cerr << "program returned " << ret << std::endl;
			}

		} else {

			FILE* fp = std::fopen(path.c_str(), "r");
			if (fp == NULL) {
				std::cerr << "fopen \"" << path << "\" failed: "
				          << std::strerror(errno) << std::endl;
				ret = 1;
			} else {
				ret = PyRun_SimpleFile(fp, path.c_str());
				if (ret != 0) {
					std::cerr << path << " returned " << ret << std::endl;
				}
				fclose(fp);
			}

		}

		if (Py_FinalizeEx()) {
			std::cerr << "Could not finalize" << std::endl;
		}
		PyMem_RawFree(program);
	}
	return ret;
}
