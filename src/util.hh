#ifndef UTIL_HH__
#define UTIL_HH__

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <random>
#include <sstream>
#include <string>

template<typename InputIt, typename OutputStream>
void join(OutputStream& stream,
          InputIt begin,
          InputIt end,
          const std::string& begin_tok = "{",
          const std::string& separator = ", ",
          const std::string& end_tok = "}") {

	stream << begin_tok;
    if(begin != end) {
        stream << *begin++;
    }

    while(begin != end) {
        stream << separator;
        stream << *begin++;
    }

    stream << end_tok;
}

template<typename InputContainer, typename OutputStream>
void join(OutputStream& stream,
          const InputContainer& container,
          const std::string& begin_tok = "{",
          const std::string& separator = ", ",
          const std::string& end_tok = "}") {
	join(stream, container.cbegin(), container.cend(), begin_tok, separator, end_tok);
}

template<typename InputIt>
std::string join(InputIt begin,
                 InputIt end,
                 const std::string& begin_tok = "{",
                 const std::string& separator = ", ",
                 const std::string& end_tok = "}") {
	std::ostringstream os;
	join(os, begin, end, begin_tok, separator, end_tok);
	return os.str();
}

template<typename InputContainer>
std::string join(InputContainer container,
                 const std::string& begin_tok = "{",
                 const std::string& separator = ", ",
                 const std::string& end_tok = "}") {
	std::ostringstream os;
	join(os, container.cbegin(), container.cend(), begin_tok, separator, end_tok);
	return os.str();
}

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

std::filesystem::path quick_tmp_copy(const std::filesystem::path& file) {
	using namespace std::filesystem;
	path tmp_file = temp_directory_path() / random_string(20);
	try {
		create_hard_link(file, tmp_file);
	} catch(const filesystem_error& e) {
		assert(static_cast<std::string>(file).substr(0, 4) != "/tmp");
		copy_file(file, tmp_file);
	}
	return tmp_file;
}

template <typename Container>
char** strings2char_pptr(const Container& args) {
	char** argv = new char*[args.size()];
	for (size_t i = 0; i < args.size(); ++i) {
		argv[i] = new char[args[i].size() + 1];
		std::copy(args[i].cbegin(), args[i].cend(), argv[i]);
		argv[i][args[i].size()] = '\0';
	}
	return argv;
}

void free_char_pptr(size_t length, char** pptr) {
	for (size_t i = 0; i < length; ++i) {
		delete[] pptr[i];
	}
	delete[] pptr;
}

#endif
