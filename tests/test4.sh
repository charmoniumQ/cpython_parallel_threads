#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

log="$(mktemp)"
py_env="env PYTHONPATH=${PWD}/cpython/Lib:${PWD}/cpython/build/lib.linux-x86_64-3.9-pydebug LD_LIBRARY_PATH=${PWD}/cpython PYTHONHOME=${PWD}/cpython"
${py_env} ./src/run_python2.exe tests/test4.py \
		  2>&1 | sort | tee "${log}"

diff "${log}" <(printf "248\n358\ntest1\ntest2\n")
