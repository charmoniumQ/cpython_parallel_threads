#ifndef DYNAMICLIB_HH__
#define DYNAMICLIB_HH__

#include <dlfcn.h>
#include <string>

#include "util.hh"

class dynamic_libs;

class dynamic_lib : protected unique_void_ptr {
private:
	void* create(const std::string& path, const std::string& name, Lmid_t ns);
	std::string name;
	friend class dynamic_libs;
public:
	dynamic_lib(const std::string& path, const std::string& name, Lmid_t ns);
	dynamic_lib(dynamic_lib&& other);
	dynamic_lib& operator=(dynamic_lib&& other);
	~dynamic_lib();

	const shared_void_ptr operator[](const std::string& symbol_name) const;
};

class dynamic_libs {
	const std::vector<dynamic_lib> dllibs;
	const std::unordered_map<std::string, const shared_void_ptr> symbols;

	dynamic_libs(std::vector<dynamic_lib>&& dllibs_, std::unordered_map<std::string, const shared_void_ptr>&& symbols_);

public:

	static dynamic_libs create(const std::vector<std::pair<std::string, std::vector<std::string>>>& paths_and_symbols);

	const shared_void_ptr operator[](const std::string& symbol_name) const;

	template <typename T>
	const T get(const std::string& symbol_name) const {
		// return reinterpret_cast<const T>(*((*this)[symbol_name]));
		return (const T) *(*this)[symbol_name];
	}
};

#endif
