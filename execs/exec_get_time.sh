#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D='orcs.d'
mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
for j in $(seq 50)
do
    for i in ${LIST_BENCHS[@]}
    do  
    #    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt "
    #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
        # pega somente o tempo real, total, já em segundos. [0.00]
        { /usr/bin/time -f "%e" echo "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} >> /dev/null" ; } 2>> ${ORCS_D}'/'${i}.txt # salvar o tempo de execucao dos benchnmars
    done
done