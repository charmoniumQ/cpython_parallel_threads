CC = g
CXX = g++
CCFLAGS += -g -Og -Wall -Wextra -fopenmp -fsanitize=address -fsanitize=undefined
CXXFLAGS += -std=c++2a $(CCFLAGS)
CFLAGS += $(CCFLAGS)
LDLIBS += -ldl

# copied from `make -p | grep '%: %.c$' -A 3`
# this way, the executable ends in a known suffix
# so I can easily identify them in .gitignore and make clean
%.exe: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.so: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -fPIC -rdynamic -shared -fno-gnu-unique -o $@ $^

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
all: src/run_sharing.exe targets/test2.so targets/test1_a.so targets/test1_b.so
