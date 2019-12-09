#ifndef UTIL_HH__
#define UTIL_HH__

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <functional>
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

template <typename In, typename Out>
std::vector<Out> map(const std::vector<In>& in, std::function<Out(const In&)> fn) {
	std::vector<Out> out;
	out.reserve(in.size());
	return std::transform(in.cbegin(), in.cend(), out.begin(), fn);
}

// this class is for wrapping C APIs in objects that manage their own memory
class unique_void_ptr {
protected:
	void* data;
public:
	unique_void_ptr(void* data_) { data = data_; };
	unique_void_ptr(const unique_void_ptr& rhs) = delete;
	unique_void_ptr(unique_void_ptr&& rhs) noexcept
		: data(std::exchange(rhs.data, nullptr)) { }
	~unique_void_ptr() { free(data); data = nullptr; }
	unique_void_ptr& operator=(const unique_void_ptr& rhs) = delete;
	unique_void_ptr& operator=(unique_void_ptr&& rhs) noexcept {
		std::swap(data, rhs.data);
		return *this;
	}
	void* operator*() { return data; }
	const void* operator*() const { return data; }
};
class shared_void_ptr {
protected:
	void* data;
public:
	shared_void_ptr() : data(nullptr) {}
	shared_void_ptr(void* data_) : data(data_) {}
	void* operator*() { return data; }
	const void* operator*() const { return data; }
};

template <typename Container>
typename Container::value_type get_and_pop_front(Container& it) {
	auto r = std::move(it.at(0));
	it.pop_front();
	return r;
}

template <typename Container>
bool try_erase(Container& container, const typename Container::key_type& key) {
	auto key_it = container.find(key);
	if (key_it != container.end()) {
		container.erase(key_it);
		return true;
	} else {
		return false;
	}
}

std::string ptr2string(void* ptr);

#endif
