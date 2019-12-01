#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

rm -f python1.so python2.so
cp /usr/lib/x86_64-linux-gnu/libpython3.7m.so.1.0 python1.so
ln python1.so python2.so

log=$(mktemp)
./src/exec_sharing.exe \
	${PWD}/src/run_python.so ${PWD}/python1.so tests/test3.py \;   \
	${PWD}/src/run_python.so ${PWD}/python2.so tests/test3.py 2>&1 \
	| sort | tee "${log}"

# same pid, different tid, same rand
test $(grep pid  "${log}" | uniq | wc -l) -eq 1
test $(grep tid  "${log}" | uniq | wc -l) -eq 2
test $(grep rand "${log}" | uniq | wc -l) -eq 1
