#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

make tests/test2.so

log=$(mktemp)
./src/exec_sharing.exe tests/test2.so \; tests/test2.so 2>&1 \
	| tee "${log}"

# 2 dtors/ctors ran
test $(grep ctor "${log}" | wc -l) -eq 2
test $(grep dtor "${log}" | wc -l) -eq 2

# same pid, different tid, same rand
test $(grep pid  "${log}" | uniq | wc -l) -eq 1
test $(grep tid  "${log}" | uniq | wc -l) -eq 2
test $(grep rand "${log}" | uniq | wc -l) -eq 1
