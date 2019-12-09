#include <iostream>
#include <stdexcept>

#include "dynamic_lib.hh"

#include <mutex>
std::mutex mutex;

// #include <chrono>
// #include <thread>
// #include <sys/syscall.h>

void* dynamic_lib::create(const std::string& path, const std::string& name, Lmid_t ns) {
	std::cout << ("create " + name + " -> " + path + "\n");
	// using namespace std::chrono_literals;
	// std::this_thread::sleep_for((syscall(SYS_gettid) / 500) * 1ms);
	void* data = dlmopen(ns, path.c_str(), RTLD_LAZY | RTLD_LOCAL);
	char* error = dlerror();
	if (error != NULL || !data)
		throw std::runtime_error{"dlmopen: " + path + ": "
				+ (error == NULL ? "NULL" : std::string{error})};
	return data;
}

dynamic_lib::dynamic_lib(const std::string& path, const std::string& name, Lmid_t ns)
	: unique_void_ptr(create(path, name, ns)), name(name) {}

dynamic_lib::dynamic_lib(dynamic_lib&& other)
	: unique_void_ptr(std::move(other)), name(other.name) {}

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
			std::cerr << "dlclose: " + name + ": "
			          << (error == NULL ? "NULL" : error) << std::endl;
	}
}

const shared_void_ptr dynamic_lib::operator[](const std::string& symbol_name) const {
	void* symbol = dlsym(data, symbol_name.c_str());
	std::cout << ("dlsym " + name + " {name: " + symbol_name + ", ptr: " + ptr2string(symbol) + "}\n");
	char* error = dlerror();
	if (error != NULL)
		throw std::runtime_error{"dlsym: " + name + ": " + symbol_name
				+ ": " + (error == NULL ? "NULL" : std::string{error})};
	return {symbol};
}

dynamic_libs dynamic_libs::create(
	const std::vector<std::pair<std::string, std::vector<std::string>>>& paths_and_symbols) {

	const std::lock_guard<std::mutex> lock(mutex);

	std::vector<dynamic_lib> dllibs;
	// dllibs.reserve(paths_and_symbols.size());
	std::unordered_map<std::string, const shared_void_ptr> symbols;

	Lmid_t ns = LM_ID_NEWLM;

	for (const auto& path_and_symbols : paths_and_symbols) {
		std::string path = quick_tmp_copy(path_and_symbols.first, 10, "foo", ".so");
		std::cout << ("dlopen for " + path_and_symbols.first + " " + path + "\n");
		dynamic_lib dlib {path, path_and_symbols.first, ns};
		if (ns == LM_ID_NEWLM && paths_and_symbols.size() > 1) {
			dlinfo(*dlib, RTLD_DI_LMID, &ns);
		}
		std::cout << ("dlopen " + path_and_symbols.first + " {ns: " + std::to_string(ns) + ", ptr: " + ptr2string(*dlib) + ", path: " + path + "}\n");

		for (const std::string& symbol : path_and_symbols.second) {
			try_erase(symbols, symbol);
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

dynamic_libs::~dynamic_libs() {
	const std::lock_guard<std::mutex> lock(mutex);
}
