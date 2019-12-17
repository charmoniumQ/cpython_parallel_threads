#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

log="$(mktemp)"
py_env="env PYTHONPATH=./cpython/Lib:./cpython/build/lib.linux-x86_64-3.9-pydebug LD_LIBRARY_PATH=./cpython PYTHONHOME=./cpython"
${py_env} ./cpython/python tests/test4.py \
		  2>&1 | sort | tee "${log}"

diff "${log}" <(printf "248\n358\ntest1\ntest2\n")

# gdb -q ./cpython/python -ex 'set env LD_LIBRARY_PATH = ./cpython' -ex 'set env PYTHONPATH = ./cpython/Lib:./cpython/build/lib.linux-x86_64-3.9-pydebug' -ex 'set env PYTHONHOME = ./cpython' -ex 'set breakpoint pending on' -ex 'b dlmopen' -ex 'r tests/test4.py' -ex 'c' -ex 'b dl-init.c:30' -ex 'c' -ex 'c' -ex 'c' -ex 'b dl-lookup.c:929' -ex 'c' -ex 'c'
