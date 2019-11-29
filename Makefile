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

.PHONY: tests
tests: src/exec_sharing.exe
	./tests/test1.sh && \
	./tests/test2.sh && \
	./tests/test3.sh && \
	true

.PHONY: all
all: src/exec_sharing.exe tests
