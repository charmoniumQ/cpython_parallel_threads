#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

log="$(mktemp)"
./src/run_python2.exe tests/test5.py \
		  2>&1 | sort | tee "${log}"
