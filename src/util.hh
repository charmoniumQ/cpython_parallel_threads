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
