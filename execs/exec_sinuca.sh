#!/bin/bash

ROOT=`pwd`
HOME_DIR="/home/rkohler/sinuca/"
SINUCA=${HOME_DIR}"./sinuca -config "${HOME_DIR}"config_examples/sandy_1core/sandy_1core.cfg -trace "
TRACE_FOLDER="/home/rkohler/traces/"
SPEC_FOLDER="spec_cpu2006/"
NANO_FOLDER="nanobench/"
cd ${TRACE_FOLDER}${SPEC_FOLDER}
SPEC_BENCHS=`ls `
cd ${ROOT}
cd ${TRACE_FOLDER}${NANO_FOLDER}
NANO_BENCHS=`ls `

SINUCA_SPEC_D="sinuca.spec.d"
SINUCA_NANO_D="sinuca.nano.d"
mkdir ${SINUCA_SPEC_D}
mkdir ${SINUCA_NANO_D}

for i in ${SPEC_BENCHS[@]}
do  
    ${SINUCA} ${TRACE_FOLDER}${SPEC_FOLDER}${i}'/'${i} "-result" ${SINUCA_SPEC_D}'/'${i}.txt &
done
for i in ${NANO_BENCHS[@]}
do  
    ${SINUCA} ${TRACE_FOLDER}${NANO_FOLDER}${i}'/'${i} "-result" ${SINUCA_NANO_D}'/'${i}.txt &
done
# /home/ricardo/sinuca/./sinuca -config /home/ricardo/sinuca/config_examples/sandy_1core/sandy_1core.cfg -trace traces/spec_cpu2006/astar.CINT.PP200M/astar.CINT.PP200M -result sinuca.d/astar.CINT.PP200M.txt
/home/rkohler/sinuca/./sinuca -config /home/rkohler/sinuca/config_examples/sandy_1core/sandy_1core.cfg -trace /home/rkohler/bin/traces/spec_cpu2006/astar.CINT.PP200M/astar.CINT.PP200M -result sinuca.spec.d/astar.CINT.PP200M.txt