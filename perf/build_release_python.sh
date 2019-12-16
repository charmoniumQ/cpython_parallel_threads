#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

# cd cpython
# if [ ! -f configure ]; then
# 	git submodule update --init
# fi
# if [ ! -f Makefile ]; then
# 	./configure --without-assertions --without-pydebug \
# 				--enable-shared --enable-optimizations --with-lto
# fi
# if [ ! -f python ]; then
# 	make -s -j
# fi
# cd ..

# py_env="env PYTHONPATH=./src:./cpython/Lib:./cpython/build/lib.linux-x86_64-3.9 LD_LIBRARY_PATH=./cpython:${LD_LIBRARY_PATH} PYTHONHOME=./cpython"
# python="${py_env} ./cpython/python"
# run_python="${py_env} ./src/run_python2.exe"
py_env="env PYTHONPATH=./src:${PYTHONPATH}"
python="${py_env} python3.8"
run_python="${py_env} ./src/run_python2.exe"
CC=clang++

if [ ! -f src/run_python2.exe ]; then
	$CC -O3 -std=c++2a \
		src/run_python2.cc src/DynamicLib.cc src/util.cc \
		-ldl -ltbb \
		-o src/run_python2.exe
fi

# py_flags="$(${python} ./cpython/python-config.py --cflags --ldflags)"
py_flags="$(python3-config --cflags --ldflags)"
if [ ! -f src/libpat.so ]; then
	$CC -O3 -std=c++2a \
		-shared -Wl,-soname,libpat -fPIC \
		${py_flags} \
		-lboost_python38 -ldl -ltbb -lboost_thread -lboost_system \
		src/libpat.cc src/DynamicLib.cc src/util.cc \
		-o src/libpat.so
fi

lscpu > lscpu
