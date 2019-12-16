#!/usr/bin/env bash

set -e -x
cd "$(dirname "$0")/.."

. ./perf/build_release_python.sh

n=1000
r=1
ms=( 1 2 4 )

# n=100000000
# r=3
# ms=( 1 2 4 8 16 32 64 )

# n=1000000000
# r=10
# ms=( 1 2 4 8 16 32 64 )

log="${0%.*}.csv"

echo "prog = ${0} n = ${n} r = ${r} commit = $(git rev-parse HEAD)" > "${log}"
echo "prog,parallelism,user_time,kernel_time,data,stack,text" >> "${log}"
for _ in `seq ${r}`
do
	for m in "${ms[@]}"
	do
		taskset="taskset -c 0-${m}"
		args="${n} ${m}"
		timeit="/usr/bin/time -o /dev/stdout --append --format="
		timeit_=",${m},%U,%S,%D,%p,%X"
		${taskset} ${timeit}pat${timeit_}    ${run_python} perf/count_pat.py    ${args} >> "${log}"
		${taskset} ${timeit}proc${timeit_}   ${python}     perf/count_proc.py   ${args} >> "${log}"
		${taskset} ${timeit}thread${timeit_} ${python}     perf/count_thread.py ${args} >> "${log}"
	done
	${taskset} ${timeit}single${timeit_} ${python}     perf/count_single.py ${args} >> "${log}"
done
