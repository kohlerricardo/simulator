#!/bin/bash

if [ -z "$1" ]
then
    echo "Basename folder empty"
    exit
else
    ROOT=`pwd`
    EXEC="orcs -t"
    TRACE_FOLDER="traces/"
    BENCHMARK_FOLDER="spec_cpu2006/"
    cd ${TRACE_FOLDER}${BENCHMARK_FOLDER}
    LIST_BENCHS=`ls`
    cd ${ROOT}  
    ORCS_D="orcs_${1}.d"

    mkdir ${ORCS_D}
    echo "Starting execute Benchmarks"
    for i in ${LIST_BENCHS[@]}
    do  
        nohup byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt"&
    #   ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i]}'/'${LIST_BENCHS[i]} -f ${ORCS_D}'/'${LIST_BENCHS[i]}.txt&
    #   ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+1]}'/'${LIST_BENCHS[i+1]} -f ${ORCS_D}'/'${LIST_BENCHS[i+1]}.txt
    #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
    #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} >> /dev/null ; } 2>> ${ORCS_D}'/'${i}.txt & # salvar o tempo de execucao dos benchnmars
    done
fi