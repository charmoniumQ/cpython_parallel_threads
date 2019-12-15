CC = clang
CXX = clang++
CFLAGS += -g -Og -Wall -Wextra
#  -fsanitize=undefined -fsanitize=address
CPPFLAGS += -std=c++2a
SHLIBFLAGS += -fPIC -rdynamic -shared
# -fno-gnu-unique
LDLIBS += -ldl -ltbb -lboost_thread -lboost_system -lpthread 
DEPS = src/DynamicLib.cc src/util.cc src/DynamicLib.hh src/util.hh


# copied from `make -p | grep '%: %.c$' -A 3`
# this way, the executable ends in a known suffix
# so I can easily identify them in .gitignore and make clean
%.exe: %.cc $(DEPS)
	$(LINK.cc) $(filter %.cc,$^) $(CFLAGS) $(LOADLIBES) $(LDLIBS) -o $@

%.so: %.cc $(DEPS) cpython/python
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(SHLIBFLAGS) -o $@ $(filter %.cc,$^)

python := env PYTHONPATH=${PWD}/cpython/Lib LD_LIBRARY_PATH=${PWD}/cpython ./cpython/python
py_flags := $(shell $(python) ./cpython/python-config.py --cflags --ldflags) -lboost_python38
src/libpat.so: src/libpat.cc $(DEPS)
	$(CXX) -shared -Wl,-soname,$(shell basename $@) -fPIC  $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(py_flags) $(LDLIBS) -o $@ $(filter %.cc,$^)
# special case, because I use Boost.Python
# https://stackoverflow.com/a/3881479/1078199
# https://stackoverflow.com/q/10968309/1078199

%.log: %.exe
	./$< | tee $@

.PHONY: %.run
%.run: %.exe
	./$<

.PHONY: %.dbg
%.dbg: %.exe
	gdb -q ./$< -ex r

.PHONY: clean
clean:
	find . -name '*.log' -delete ; \
	find . -name '*.exe' -delete ; \
	find . -name '*.o'   -delete ; \
	find . -name '*.so'  -delete ; \
	true

.PHONY: python_clean
python_clean:
	cd cpython && $(MAKE) clean && rm -f python && cd ..

cpython/python:
	cd cpython && \
	git submodule update --init && \
	./configure --enable-shared --with-pydebug && \
	$(MAKE) -s && \
	cd ..

.PHONY: tests
tests: src/exec_sharing.exe src/run_python2.so src/run_python2.exe src/libpat.so cpython/python
	./tests/test1.sh && \
	./tests/test2.sh && \
	./tests/test3.sh && \
	./tests/test4.sh && \
	true

.PHONY: all
all: exec_sharing.exe tests
