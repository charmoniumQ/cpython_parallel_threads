#ifndef DYNAMICLIB_HH__
#define DYNAMICLIB_HH__

#include <string>

class DynamicLib {
	void* dllib = NULL;
public:

	DynamicLib(const std::string& path);

	~DynamicLib();

	void* operator[](const std::string& symbol_name);

	template <typename T>
	T get(const std::string& symbol_name) {
		return reinterpret_cast<T>((*this)[symbol_name]);
	}

};

#endif
