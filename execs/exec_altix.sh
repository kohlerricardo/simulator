#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D="orcs_emc.d"
mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
for i in ${LIST_BENCHS[@]}
do  
#   byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt "
   byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} >> ${i}.txt "
#   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
done
