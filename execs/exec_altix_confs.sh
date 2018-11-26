#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D=("C1_ROB_168_LLC_4M_RAM_200" 
        "C2_ROB_384_LLC_4M_RAM_200"
        "C3_ROB_168_LLC_512K_RAM_200"
        "C4_ROB_168_LLC_4M_RAM_800"
        "C5_ROB_384_LLC_512K_RAM_200"
        "C6_ROB_384_LLC_512K_RAM_800")
# mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
for jj in ${ORCS_D[@]}
do
        mkdir ${jj}'.d'
        for i in ${LIST_BENCHS[@]}
        do  
        # echo "${ROOT}'/'${jj}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${jj}.d'/'${i}.txt "
        byobu new-window "${ROOT}'/'${jj}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${jj}.d'/'${i}.txt "
        #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
        done
done

