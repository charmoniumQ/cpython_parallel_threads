#include <chrono>
#include <cstring>
#include <filesystem>
#include <future>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <sys/wait.h>

#define PY_VERSION_HEX 0x03700000
#include <boost/python.hpp>

#include "DynamicLib.cc"
#include "util.hh"

namespace py = boost::python;

class ProcessAsThread {
private:
	typedef int(*main_method)(int, char**);
	int argc;
	char** argv;
	DynamicLib lib;
	main_method main;
	std::future<int> return_code;
public:

	ProcessAsThread(const py::list& cmd)
		: argc(len(cmd))
		, argv(init_argv(argc, cmd))
		, lib({quick_tmp_copy(argv[0])})
		, main(init_main(lib))
		, return_code(init_return_code(main, argc, argv))
	{}

	char** init_argv(int argc, const py::list& cmd) {
		char** out = new char*[argc];
		for (int i = 0; i < argc; ++i) {
			const char* arg = py::extract<char const*>(cmd[i]);
			int arg_size = strlen(arg) + 1;
			out[i] = new char[arg_size];
			memcpy(out[i], arg, arg_size);
		}
		return out;
	}

	static main_method init_main(DynamicLib& lib) {
		return lib.get<main_method>("main");
	}

	static std::future<int> init_return_code(main_method& main, int argc, char** argv) {
		return std::async(main, argc, argv);
	}

	~ProcessAsThread() {
		for (int i = 0; i < argc; ++i) {
			delete[] argv[i];
		}
		delete[] argv;
	}

	/*
	  Waits for timeout and then gets return_code
	  Wait can be 0, a positive real, or infinity.
	 */
	std::optional<int> wait(float timeout) {
		assert(timeout > 0);
		auto status = std::isinf(timeout)
			? ({return_code.wait(); std::future_status::ready;})
			: return_code.wait_for(std::chrono::microseconds{static_cast<long>(timeout*1e6)})
			;
		switch(status) {
		case std::future_status::deferred:
			throw std::runtime_error{"Thread should not be deferred"};
		case std::future_status::timeout:
			return {};
		case std::future_status::ready:
			return return_code.get();
		}
	}
};

// https://stackoverflow.com/a/26644530/1078199
namespace detail {

	/// @brief Type trait that determines if the provided type is
	///        an optional.
	template <typename>
	struct is_optional : std::false_type {};
	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};

	/// @brief ResultConverter model that converts a optional object to
	///        Python None if the object is empty (i.e. none) or defers
	///        to Boost.Python to convert object to a Python object.
	template <typename T> struct to_python_optional {

		/// @brief Only supports converting Optional types.
		/// @note This is checked at runtime.

		bool convertible() const { return detail::is_optional<T>::value; }

		/// @brief Convert optional object to Python None or a
		///        Boost.Python object.
		PyObject* operator()(const T& obj) const {
			py::object result =
				obj                    // If optional has a value, then
				? py::object(*obj) // defer to Boost.Python converter.
				: py::object()     // Otherwise, return Python None.
				;

			// The python::object contains a handle which functions as
			// smart-pointer to the underlying PyObject.  As it will go
			// out of scope, explicitly increment the PyObject's reference
			// count, as the caller expects a non-borrowed (i.e. owned) reference.
			return py::incref(result.ptr());
		}

		/// @brief Used for documentation.
		const PyTypeObject* get_pytype() const { return 0; }

	};

}

/// @brief Converts a optional to Python None if the object is
///        equal to none.  Otherwise, defers to the registered
///        type converter to returs a Boost.Python object.
struct return_optional {
	template <class T> struct apply {
		// The to_python_optional ResultConverter only checks if T is convertible
		// at runtime.  However, the following MPL branch cause a compile time
		// error if T is not a optional by providing a type that is not a
		// ResultConverter model.
		typedef typename std::enable_if<
			detail::is_optional<T>::value,
			detail::to_python_optional<T>
			>::type type;
	};
};


BOOST_PYTHON_MODULE(libpat) {
	py::class_<ProcessAsThread, boost::noncopyable>(
			"ProcessAsThread",
			py::init<py::list>())
		.def("wait", &ProcessAsThread::wait,
			 py::return_value_policy<return_optional>())
	;
}
