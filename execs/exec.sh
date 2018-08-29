#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D="orcs.d"
mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
for i in ${LIST_BENCHS[@]}
do  
    ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt &
    # sleep 5
done