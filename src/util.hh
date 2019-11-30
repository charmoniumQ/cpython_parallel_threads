#ifndef UTIL_HH__
#define UTIL_HH__

#include <sstream>

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

#endif
