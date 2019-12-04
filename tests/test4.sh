#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

prog="import libpat
s = [
    libpat.ProcessAsThread(['./tests/test1_a.so']),
    libpat.ProcessAsThread(['./tests/test1_b.so']),
    libpat.ProcessAsThread(['python', '-c', 'print(248)']),
]
print(s[0].wait(1))
print(s[1].wait(1))"
log="$(mktemp)"
PYTHONPATH=src python -c "${prog}" \
		  2>&1 | sort | tee "${log}"

diff "${log}" <(printf "0\n0\n248\ntest1\ntest2\n")
