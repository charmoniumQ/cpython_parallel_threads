CC = clang
CXX = clang++
CFLAGS += -g -Og -Wall -Wextra
#  -fsanitize=undefined -fsanitize=address
CPPFLAGS += -std=c++2a
SHLIBFLAGS += -fPIC -rdynamic -shared
# -fno-gnu-unique
LDLIBS += -ldl -ltbb -lboost_thread -lboost_system -lpthread 
DEPS = src/dynamic_lib.cc src/util.cc src/dynamic_lib.hh src/util.hh cpython/python src/python_so_path.hh

# copied from `make -p | grep '%: %.c$' -A 3`
# this way, the executable ends in a known suffix
# so I can easily identify them in .gitignore and make clean
%.exe: %.cc $(DEPS)
	$(LINK.cc) $(filter %.cc,$^) $(CFLAGS) $(LOADLIBES) $(LDLIBS) -o $@

%.so: %.cc $(DEPS)
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
	find src   -name '*.{log,o,exe,so}' -delete ; \
	find tests -name '*.{log,o,exe,so}' -delete ; \
	true

.PHONY: python_clean
cpython_clean:
	cd cpython && $(MAKE) clean && rm -f python && cd ..

cpython/Makefile:
	cd cpython && \
	git submodule update --init && \
	./configure --enable-shared --with-pydebug && \
	cd ..

cpython/python: cpython/Makefile
	cd cpython && \
	$(MAKE) -s -j 2 && \
	cd ..

.PHONY: tests
tests: src/exec_sharing.exe src/run_python2.so src/run_python2.exe src/libpat.so
	./tests/test1.sh && \
	./tests/test2.sh && \
	./tests/test3.sh && \
	./tests/test4.sh && \
	true

.PHONY: all
all: exec_sharing.exe tests
