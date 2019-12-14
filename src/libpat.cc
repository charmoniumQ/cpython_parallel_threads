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

#include <boost/thread/sync_queue.hpp>
#define PY_VERSION_HEX 0x03700000
#include <boost/python.hpp>

#include "DynamicLib.hh"
#include "util.hh"

std::string python_so = "/usr/lib/x86_64-linux-gnu/libpython3.7m.so";

namespace py = boost::python;
namespace bc = boost::concurrent;

class ProcessAsThread {
private:
	int argc;
	char** argv;
	DynamicLib lib;
	std::future<int> return_code;
public:

	ProcessAsThread(const py::list& cmd)
		: argc(len(cmd))
		, argv(init_argv(argc, cmd))
		, lib({quick_tmp_copy(argv[0], 10, "foo", ".so")})
		, return_code(std::async(lib.get<int (*)(int, char**)>("main"), argc, argv))
	{ }

	static char** init_argv(int argc, const py::list& cmd) {
		char** out = new char*[argc];
		for (int i = 0; i < argc; ++i) {
			char const* arg = py::extract<char const*>(cmd[i]);
			int arg_size = strlen(arg) + 1;
			out[i] = new char[arg_size];
			memcpy(out[i], arg, arg_size);
		}
		return out;
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
	py::object wait(float timeout) {
		assert(timeout > 0);
		auto status = std::isinf(timeout)
			? ({return_code.wait(); std::future_status::ready;})
			: return_code.wait_for(
								   // remember timeout is a real number of seconds
								   std::chrono::microseconds{static_cast<long>(timeout*1e6)})
			;
		switch(status) {
		case std::future_status::deferred:
			throw std::runtime_error{"Thread should not be deferred"};
		case std::future_status::timeout:
			return py::object{};
		case std::future_status::ready:
			return py::long_{return_code.get()};
		}
	}
};

class PythonProcessAsThread {
private:
	int argc;
	wchar_t** argv;
	DynamicLib lib;
	std::future<int> return_code;
public:

	PythonProcessAsThread(const py::list& cmd)
		: argc(len(cmd))
		, argv(init_argv(argc, cmd))
		, lib({quick_tmp_copy(python_so, 10, "foo", ".so")})
		, return_code(std::async(lib.get<int (*)(int, wchar_t**)>("Py_Main"), argc, argv))
	{ }

	static wchar_t** init_argv(int argc, const py::list& cmd) {
		wchar_t** out = new wchar_t*[argc];
		for (int i = 0; i < argc; ++i) {
			char const* arg = py::extract<char const*>(cmd[i]);
			int arg_size = strlen(arg) * 4;
			out[i] = new wchar_t[arg_size];
			mbstowcs(out[i], arg, arg_size);
		}
		return out;
	}

	~PythonProcessAsThread() {
		for (int i = 0; i < argc; ++i) {
			delete[] argv[i];
		}
		delete[] argv;
	}

	/*
	  Waits for timeout and then gets return_code
	  Wait can be 0, a positive real, or infinity.
	 */
	py::object wait(float timeout) {
		assert(timeout > 0);
		auto status = std::isinf(timeout)
			? ({return_code.wait(); std::future_status::ready;})
			: return_code.wait_for(
								   // remember timeout is a real number of seconds
								   std::chrono::microseconds{static_cast<long>(timeout*1e6)})
			;
		switch(status) {
		case std::future_status::deferred:
			throw std::runtime_error{"Thread should not be deferred"};
		case std::future_status::timeout:
			return py::object{};
		case std::future_status::ready:
			return py::long_{return_code.get()};
		}
	}
};

class Queue {
private:
	bc::sync_queue<py::object> _queue;

public:
	void push(py::object obj) {
		// TODO: assess if I need this
		py::incref(obj.ptr());
		// I think I need to make sure Python doesn't delete this,
		// even if it is not referenced in user code.
		// Consider it referenced here.
		_queue.wait_push(obj);
	}
	py::object pop() {
		// TODO: assess if I need this
		// py::decref(obj.ptr());
		// I think I already increfed on push
		// so obj's ref_count = references_already_in_user_code + 1
		// and I am going to return it.
		// So I think this is good.
		return _queue.pull();
	}
	py::object try_pop() {
		py::object obj;
		bool success = _queue.try_pull(obj) == bc::queue_op_status::success;
		if (success) {
			// TODO: assess if I need this
			// py::decref(obj.ptr());
			// I think I already increfed on push
			// so obj's ref_count = references_already_in_user_code + 1
			// and I am going to return it.
			// So I think this is good.
			return obj;
		}
		else {
			return py::object{};
		}
	}
};

BOOST_PYTHON_MODULE(libpat) {
	py::class_<ProcessAsThread, boost::noncopyable>(
			"ProcessAsThread",
			py::init<py::list>())
		.def("wait", &ProcessAsThread::wait)
	;

	py::class_<PythonProcessAsThread, boost::noncopyable>(
			"PythonProcessAsThread",
			py::init<py::list>())
		.def("wait", &PythonProcessAsThread::wait)
	;

	py::class_<Queue, boost::noncopyable>("Queue", py::init<>())
		.def("push", &Queue::push)
		.def("pop", &Queue::pop)
		.def("try_pop", &Queue::try_pop)
	;
}
