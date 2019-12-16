#!/usr/bin/env bash

cd "$(dirname "$0")/.."

. ./perf/build_release_python.sh

# r=1
# ms=( 1 2 4 )
r=5
ms=( 1 2 4 8 16 32)

log="${0%.*}.csv"

echo "prog = ${0} r = ${r} commit = $(git rev-parse HEAD)" > "${log}"
echo "prog,parallelism,user_time,kernel_time,data,stack,text" >> "${log}"
for _ in `seq ${r}`
do
	for m in "${ms[@]}"
	do
		taskset="taskset -c 0-${m}"
		args="${m}"
		timeit="/usr/bin/time -o /dev/stdout --append --format="
		timeit_=",${m},%U,%S,%D,%p,%X"
		${taskset} ${timeit}pat${timeit_}    ${run_python} perf/noop_pat.py    ${args} >> "${log}" || true
		${taskset} ${timeit}proc${timeit_}   ${python}     perf/noop_proc.py   ${args} >> "${log}"
		${taskset} ${timeit}thread${timeit_} ${python}     perf/noop_thread.py ${args} >> "${log}"
		rm -rf /tmp/foo
		mkdir /tmp/foo
	done
	${taskset} ${timeit}single${timeit_}    ${python} perf/noop_single.py    ${args} >> "${log}"
done
