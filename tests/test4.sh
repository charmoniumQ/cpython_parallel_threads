#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

log="$(mktemp)"
./src/run_python2.exe tests/test4.py \
		  2>&1 | sort | tee "${log}"

diff "${log}" <(printf "248\n358\ntest1\ntest2\n")
