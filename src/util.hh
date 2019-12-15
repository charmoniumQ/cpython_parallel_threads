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
          const InputIt& end,
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

template<typename Container, typename OutputStream>
void join(OutputStream& stream,
          const Container& container,
          const std::string& begin_tok = "{",
          const std::string& separator = ", ",
          const std::string& end_tok = "}") {
	join(stream, container.cbegin(), container.cend(), begin_tok, separator, end_tok);
}

template<typename InputIt>
std::string join(InputIt begin,
                 const InputIt& end,
                 const std::string& begin_tok = "{",
                 const std::string& separator = ", ",
                 const std::string& end_tok = "}") {
	std::ostringstream os;
	join(os, begin, end, begin_tok, separator, end_tok);
	return os.str();
}

template<typename Container>
std::string join(const Container& container,
                 const std::string& begin_tok = "{",
                 const std::string& separator = ", ",
                 const std::string& end_tok = "}") {
	std::ostringstream os;
	join(os, container.cbegin(), container.cend(), begin_tok, separator, end_tok);
	return os.str();
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

std::string random_string(size_t length);
std::filesystem::path quick_tmp_copy(
	 const std::filesystem::path& file,
	 size_t size = 15,
	 std::string marker = "foo",
	 std::string suffix = "");
void free_char_pptr(size_t length, char** pptr);
std::string ptr2string(void* ptr);

#endif
