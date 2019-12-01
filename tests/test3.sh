#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

# test -c STRING and FILE execution modes
log=$(mktemp)
./src/exec_sharing.exe \
	src/run_python2.so           tests/test3.py   \;   \
	src/run_python2.so -c "$(cat tests/test3.py)" 2>&1 \
	| sort | tee "${log}"

# same pid, different tid, same rand
test $(grep pid  "${log}" | uniq | wc -l) -eq 1
test $(grep tid  "${log}" | uniq | wc -l) -eq 2
test $(grep rand "${log}" | uniq | wc -l) -eq 1
