#include <deque>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>

#include "../src/DynamicLib.cc"

template <typename T>
T get_and_pop_front(std::deque<T>& it) {
	T r = it[0];
	it.pop_front();
	return r;
}

int main(int argc, char** argv) {
	int ret = 0;
	std::deque<std::string> args {argv, argv + argc};
	std::string prog_name = get_and_pop_front(args);
	if (args.size() < 2) {
		std::cerr << "Usage: " << prog_name
		          << " <python so> (<program> | -c <string>)" << std::endl;
		ret = 1;
	} else {
		DynamicLib lib {get_and_pop_front(args)};
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
				ret = PyRun_SimpleString(prog.c_str());
			} else {
				FILE* fp = std::fopen(path.c_str(), "r");
				if (fp == NULL) {
					std::cerr << "fopen \"" << path << "\" failed: "
					          << strerror(errno) << std::endl;
					ret = 1;
				} else {
					ret = PyRun_SimpleFile(fp, path.c_str());
					if (ret != 0) {
						std::cerr << path << " returned " << ret << std::endl;
					}
					fclose(fp);
				}
			}
			ret = Py_FinalizeEx();
			if (ret) {
				std::cerr << "Could not finalize" << std::endl;
			}
			PyMem_RawFree(program);
		}
	}
	return ret;
}
