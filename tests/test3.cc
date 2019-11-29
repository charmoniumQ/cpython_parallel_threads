#include <vector>
#include <iostream>
#include <string>
#include <cstdio>

#include "../src/DynamicLib.cc"

int main(int argc, const char* const* argv) {
	int ret = 0;
	std::vector<std::string> args {argv, argv + argc};

	DynamicLib lib {std::move(args[1])};
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

	if (args.size() != 3) {
		std::cerr << "Incorrect usage" << std::endl;
		ret = 1;
	} else {

		std::string path = std::move(args[2]);

		wchar_t* const program = Py_DecodeLocale(args[0].c_str(), NULL);
		if (program == NULL) {
			std::cerr << "OOM" << std::endl;
			ret = 1;
		} else {

			Py_SetProgramName(program);
			Py_Initialize();

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

				ret = Py_FinalizeEx();
				if (ret) {
					std::cerr << "Could not finalize" << std::endl;
				}

				fclose(fp);
			}

			PyMem_RawFree(program);
		}
	}

	return ret;
}
