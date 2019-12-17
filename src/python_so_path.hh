#include <string>

static std::string get_python_so() {
	// TODO: get this value programatically by
	// $ sbin/ldconfig -p | grep -P -o '\S*libpython3....so$'
	// overridable by env var
	return "cpython/libpython3.8d.so";
}
