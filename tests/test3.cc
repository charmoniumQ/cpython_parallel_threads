#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdio>

int main(int argc, const char* const* argv) {
	int ret = 0;
	std::vector<std::string> args {argv, argv + argc};

	if (args.size() != 2) {
		std::cerr << "Incorrect usage" << std::endl;
		ret = 1;
	} else {

		std::string path = std::move(args[1]);

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
