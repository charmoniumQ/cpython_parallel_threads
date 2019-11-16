CC = clang
CXX = clang++
CFLAGS += -g -Og -Wall -Wextra
#-fsanitize=address -fsanitize=undefined
CPPFLAGS += -std=c++2a
SHLIBFLAGS += -fPIC -rdynamic -shared
# -fno-gnu-unique
LDLIBS += -ldl -ltbb

# copied from `make -p | grep '%: %.c$' -A 3`
# this way, the executable ends in a known suffix
# so I can easily identify them in .gitignore and make clean
%.exe: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

targets/python_run.so: targets/python_run.cc
	$(CXX) $(CPPFLAGS) $(TARGET_ARCH) $(SHLIBFLAGS) $(shell python3.7-config --ldflags) $(shell python3.7-config --cflags) -o $@ $^

%.so: %.cc
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(SHLIBFLAGS) -o $@ $^

%.log: %.exe $(lastword $(MAKEFILE_LIST))
	./$< | tee $@

.PHONY: %.run
%.run: %.exe
	./$<

.PHONY: %.dbg
%.dbg: %.exe
	gdb -q ./$< -ex r

.PHONY: clean
clean:
	find . -name '*.log' -print0 | xargs -0 rm -f ; \
	find . -name '*.exe' -print0 | xargs -0 rm -f ; \
	find . -name '*.o'   -print0 | xargs -0 rm -f ; \
	find . -name '*.so'  -print0 | xargs -0 rm -f ; \
	true

.PHONY: all
all: src/run_sharing.exe targets/test2.so targets/test1_a.so targets/test1_b.so targets/python_run.so
