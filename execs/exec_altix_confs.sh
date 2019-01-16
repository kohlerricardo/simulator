#!/bin/bash

ROOT='/home/ricardo'
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D=(limit_op_rh.d all_op_rh.d limit_op_all_miss.d all_op_all_miss.d)
# mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
# for i in ${ORCS_D[@]}
# do
# cd /home/ricardo/${i}
# make clean && make -j 4 all
# done
# cd -
for jj in ${LIST_BENCHS[@]}
do
        for ((i=0;i<${#ORCS_D[@]};i+=4))
        do  
        # mkdir ${i}'.d'
        # echo "${ROOT}'/'${jj}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${jj}.d'/'${i}.txt "
        ${ROOT}'/'${ORCS_D[i]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${jj}'/'${jj} -f '/home/ricardo/bin/'${ORCS_D[i]}'/'${jj}.txt& 
        ${ROOT}'/'${ORCS_D[i+1]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${jj}'/'${jj} -f '/home/ricardo/bin/'${ORCS_D[i+1]}'/'${jj}.txt&
        ${ROOT}'/'${ORCS_D[i+2]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${jj}'/'${jj} -f '/home/ricardo/bin/'${ORCS_D[i+2]}'/'${jj}.txt&
        ${ROOT}'/'${ORCS_D[i+3]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${jj}'/'${jj} -f '/home/ricardo/bin/'${ORCS_D[i+3]}'/'${jj}.txt 
        #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
        done
done

