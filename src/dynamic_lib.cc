#include <iostream>
#include <mutex>
#include <stdexcept>
#include <dlfcn.h>

#include "dynamic_lib.hh"

std::mutex dlfcn_mutex;
void* dynamic_lib::create(const std::string& path) {
	void* data = dlopen(path.c_str(), RTLD_NOW | RTLD_DEEPBIND);
	char* error = dlerror();
	if (error != NULL || !data)
		throw std::runtime_error{
			"dlopen(): " + (error == NULL ? "NULL" : std::string{error})};
	return data;
}

dynamic_lib::dynamic_lib(const std::string& path) : unique_void_ptr(create(path)), name(path) {}
dynamic_lib::dynamic_lib(dynamic_lib&& other) : unique_void_ptr(std::move(other)), name(other.name) {}
dynamic_lib& dynamic_lib::operator=(dynamic_lib&& other) {
	std::swap(*this, other);
	return *this;
}
dynamic_lib::~dynamic_lib() {
	if (data) {
		int ret = dlclose(data);
		data = nullptr;
		char* error = dlerror();
		if (error != NULL || ret)
			std::cerr
				<< "dlclose(): " << (error == NULL ? "NULL" : error) << std::endl;
	}
}

const shared_void_ptr dynamic_lib::operator[](const std::string& symbol_name) const {
	void* symbol = dlsym(data, symbol_name.c_str());
	char* error = dlerror();
	if (error != NULL)
		throw std::runtime_error{
			"dlsym(): " + (error == NULL ? "NULL" : std::string{error})};
	return {symbol};
}

dynamic_libs dynamic_libs::create(
	const std::vector<std::pair<std::string, std::vector<std::string>>>& paths_and_symbols) {

	std::lock_guard<std::mutex> guard {dlfcn_mutex};

	std::vector<dynamic_lib> dllibs;
	// dllibs.reserve(paths_and_symbols.size());
	std::unordered_map<std::string, const shared_void_ptr> symbols;

	for (const auto& path_and_symbols : paths_and_symbols) {
		dynamic_lib dlib {quick_tmp_copy(path_and_symbols.first, 10, "foo", ".so")};
		for (const std::string& symbol : path_and_symbols.second) {
			symbols.erase(symbol);
			// emplace lets us construct a const shared_void_ptr
			symbols.emplace(symbol, dlib[symbol]);
		}
		dllibs.push_back(std::move(dlib));
	}

	return {std::move(dllibs), std::move(symbols)};
}

dynamic_libs::dynamic_libs(std::vector<dynamic_lib>&& dllibs_, std::unordered_map<std::string, const shared_void_ptr>&& symbols_)
	: dllibs(std::move(dllibs_)), symbols(std::move(symbols_)) {}

const shared_void_ptr dynamic_libs::operator[](const std::string& symbol_name) const {
	return symbols.at(symbol_name);
}
