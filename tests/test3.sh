#!/usr/bin/env bash

set -e -x
cd $(dirname "$0")/..

CFLAGS="$(python3.7-config --ldflags) $(python3.7-config --cflags)"
CFLAGS="${CFLAGS}" make tests/test3.so
cp tests/test3.so tests/test3_b.so

log=$(mktemp)
./src/exec_sharing.exe \
	tests/test3.so   tests/test3.py \; \
	tests/test3_b.so tests/test3.py 2>&1 \
	| sort | tee "${log}"
