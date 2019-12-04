#include "util.hh"

std::string random_string(size_t length) {
	static auto& chrs = "0123456789"
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static std::random_device rd;
	static std::uniform_int_distribution<size_t> pick{0, sizeof(chrs) - 1};

	// Note that {length, '\0'} tries to use an initializer list
	// which is not what I want
	std::string str(length, '\0');
    std::generate_n(str.begin(), length, []{ return chrs[pick(rd)]; });
	return str;
}

#include <iostream>
std::filesystem::path quick_tmp_copy(const std::filesystem::path& file, size_t size, std::string marker, std::string suffix) {
	using namespace std::filesystem;
	path dir = temp_directory_path() / marker;
	create_directory(dir);
	path tmp_file = dir / (random_string(size) + suffix);
	try {
		create_hard_link(file, tmp_file);
	} catch(const filesystem_error& e) {
		assert(static_cast<std::string>(file).substr(0, 4) != "/tmp");
		copy_file(file, tmp_file);
	}
	return tmp_file;
}

void free_char_pptr(size_t length, char** pptr) {
	for (size_t i = 0; i < length; ++i) {
		delete[] pptr[i];
	}
	delete[] pptr;
}
