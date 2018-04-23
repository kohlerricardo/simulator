#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="nanobench/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
NANO_D="nano.d"
mkdir ${NANO_D}
for i in ${LIST_BENCHS[@]}
do  
    echo ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt
    # sleep 5
done
