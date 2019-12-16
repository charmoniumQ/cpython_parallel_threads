#!/usr/bin/env bash

cd "$(dirname "$0")/.."

. ./perf/build_release_python.sh

# n=1000
# r=1
# ms=( 1 2 4 )

n=400000000
r=5
ms=( 1 2 4 8 16 32 64 72 )

# n=1000000000
# r=10
# ms=( 1 2 4 8 16 32 64 )

log="${0%.*}.csv"

echo "prog = ${0} n = ${n} r = ${r} commit = $(git rev-parse HEAD)" > "${log}"
echo "prog,parallelism,total_time,user_time,kernel_time" >> "${log}"
for _ in `seq ${r}`
do
	for m in "${ms[@]}"
	do
		taskset="taskset -c 0-${m}"
		args="${n} ${m}"
		timeit="perf stat"
		timeit_="grep -Po '[0-9.]*(?= seconds)' | tr '\n' ','"
		echo -e -n "\npat,${m}," >> "${log}"
		${taskset} ${timeit} ${run_python} perf/count_pat.py    ${args} 2>&1 | grep -Po '[0-9.]*(?= seconds)' | tr '\n' ',' >> "${log}" || true
		echo -e -n "\nproc,${m}," >> "${log}"
		${taskset} ${timeit} ${python}     perf/count_proc.py   ${args} 2>&1 | grep -Po '[0-9.]*(?= seconds)' | tr '\n' ',' >> "${log}"
		echo -e -n "\nthread,${m}," >> "${log}"
		${taskset} ${timeit} ${python}     perf/count_thread.py ${args} 2>&1 | grep -Po '[0-9.]*(?= seconds)' | tr '\n' ','  >> "${log}"
		rm -rf /tmp/foo
		mkdir /tmp/foo
	done
	echo -e -n "\nsingle,1," >> "${log}"
	${taskset} ${timeit} ${python}     perf/count_single.py ${args} 2>&1 | grep -Po '[0-9.]*(?= seconds)' | tr '\n' ',' >> "${log}"
done
echo "" >> "${log}"
