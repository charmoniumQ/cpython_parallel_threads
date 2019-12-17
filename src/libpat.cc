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
#include <iostream>

#include <boost/thread/sync_queue.hpp>
#define PY_VERSION_HEX 0x03700000
#include <boost/python.hpp>

#include "dynamic_lib.hh"
#include "util.hh"
#include "python_so_path.hh"

namespace py = boost::python;
namespace bc = boost::concurrent;

class ProcessAsThread {
private:
	int argc;
	char** argv;
	dynamic_libs lib;
	std::future<int> return_code;
public:

	// I don't own cmd or its elements.
	// I just borrow them for the duration of this constructor.
	// I copy the data out of them and never touch them again.
	// So no need to incref/decref cmd or its elements.
	ProcessAsThread(const py::list& cmd)
		: argc(len(cmd))
		, argv(init_argv(argc, cmd))
		, lib(dynamic_libs::create({{argv[0], {"main"}}}))
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
	char** argv;
	std::future<int> return_code;
	
public:

	PythonProcessAsThread(const py::list& cmd)
		: argc(len(cmd))
		, argv(init_argv(argc, cmd))
		, return_code(std::async(run, argc, argv))
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

	static int run(int argc, char** argv) {
		int ret = 0;
		dynamic_libs lib = dynamic_libs::create({
			{get_python_so(), {
				"Py_BytesMain",
				"Py_InitializeEx",
				"Py_FinalizeEx",
			}},
		});

		auto Py_InitializeEx = lib.get
			<void(*)(int)>
			("Py_InitializeEx");
		auto Py_FinalizeEx = lib.get
			<int(*)()>
			("Py_FinalizeEx");
		auto Py_BytesMain = lib.get
			<int(*)(int, char**)>
			("Py_BytesMain");

		Py_InitializeEx(0);
		ret |= Py_BytesMain(argc, argv);
		ret |= Py_FinalizeEx();

		return ret;
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
	using queue_t = bc::sync_queue<py::object>;
	queue_t* queue;
	const bool owned;

public:

	// Note on thread-safety:
	// If this queue is shared between threads
	// deepcopy objects before pushing them.
	// This way, all objects are referenced from only one thread.
	void push(py::object obj) {
		// You can keep your existing references to obj.
		// But I need one too.
		py::incref(obj.ptr());
		// This ensures Python doesn't delete it
		// even when user-code has no references to obj.
		// (I still do)

		// In a multi-threaded environment, refcount will be equal to one
		// because you made a copy, pushed it, (implicitly) threw it away, and decref'ed.

		queue->wait_push(obj);
	}

	py::object pop() {
		// Returning an owned object.
		// It is live because I inref'ed when I inserted.
		// Python is responsible for calling decref.
		// I think this happens when the var holding this result in user-code goes out of scope
		return queue->pull();
	}

	py::object try_pop() {
		py::object obj;
		bool success = queue->try_pull(obj) == bc::queue_op_status::success;
		if (success) {
			// See reference-semantics of pop
			return obj;
		}
		else {
			return py::object{};
		}
	}

	std::string handle() {
		return ptr2string(queue);
	}

	static Queue from_handle(const std::string& re) {
		return {reinterpret_cast<queue_t*>(std::stoi(re))};
	}

	Queue() : queue{new queue_t{}}, owned{true} {}

	~Queue() {
		if (owned) {
			delete queue;
		}
	}

private:
	Queue(queue_t* _queue) : queue{_queue}, owned{false} {}
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
		.add_property("handle", &Queue::handle)
		.def("from_handle", &Queue::from_handle).staticmethod("from_handle")
	;
}
