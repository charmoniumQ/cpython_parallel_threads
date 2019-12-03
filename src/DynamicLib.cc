#include <iostream>
#include <string>
#include <stdexcept>
#include <dlfcn.h>
#include "util.hh"

class DynamicLib {
	void* dllib = NULL;
public:

	DynamicLib(const std::string& path) {
		char* error;
		dllib = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
		if ((error = dlerror()) != NULL || !dllib)
			throw std::runtime_error{
				"dlopen(): " + (error == NULL ? "NULL" : std::string{error})};
	}

	~DynamicLib() {
		char* error;
		int ret = dlclose(dllib);
		if ((error = dlerror()) != NULL || ret)
			std::cerr
				<< "dlclose(): " << (error == NULL ? "NULL" : error)
				<< std::endl;
	}

	void* operator[](const std::string& symbol_name) {
		char* error;
		void* symbol = dlsym(dllib, symbol_name.c_str());
		if ((error = dlerror()) != NULL)
			throw std::runtime_error{
				"dlsym(): " + (error == NULL ? "NULL" : std::string{error})};
		return symbol;
	}

	template <typename T>
	T get(const std::string& symbol_name) {
		return reinterpret_cast<T>((*this)[symbol_name]);
	}

};
