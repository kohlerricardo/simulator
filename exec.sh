#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
for i in ${LIST_BENCHS[@]}
do  
    echo "==============================================================================================">>${i}.txt
    echo ${i} >>${i}.txt
    ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} >>${i}.txt
    echo "==============================================================================================">>${i}.txt
    sleep 5
done
