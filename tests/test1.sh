#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

make tests/test1_a.so tests/test1_b.so

log="$(mktemp)"
./src/exec_sharing.exe tests/test1_a.so \; tests/test1_b.so \
	2>&1 | sort | tee "${log}"

# check output
diff "${log}" <(printf "test1\ntest2\n") 
