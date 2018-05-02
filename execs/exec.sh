#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=`ls `
cd ${ROOT}
ORCS_D="new_orcs.d"
mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
for i in ${LIST_BENCHS[@]}
do  
    ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt
    # sleep 5
done
# /home/ricardo/sinuca/./sinuca -config /home/ricardo/sinuca/config_examples/sandy_1core/sandy_1core.cfg -trace /home/ricardo/ProgramasMestrado/traces/spec_cpu2006/astar.CINT.PP200M/astar.CINT.PP200M
# /home/ricardo/sinuca/./sinuca -config /home/ricardo/sinuca/config_examples/sandy_1core/sandy_1core.cfg -trace traces/spec_cpu2006/astar.CINT.PP200M/astar.CINT.PP200M -result sinuca.d/astar.CINT.PP200M.txt
