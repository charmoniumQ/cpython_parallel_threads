%.exe: %.cc
	$(LINK.cc) -g -ldl -o $@ $^

%.so: %.cc
	$(CXX) -g -fPIC -rdynamic -shared -o $@ $^

clean:
	find . -name '*.exe' -delete ; \
	find . -name '*.so'  -delete ; \
	true

run: lib.so test.exe
	./test.exe
