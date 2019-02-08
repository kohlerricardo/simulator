#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
LIST_BENCHS=(astar.CINT.PP200M bwaves.CFP.PP200M bzip2.CINT.PP200M cactusADM.CFP.PP200M calculix.CFP.PP200M dealII.CFP.PP200M 
            gamess.CFP.PP200M gcc.CINT.PP200M GemsFDTD.CFP.PP200M gobmk.CINT.PP200M gromacs.CFP.PP200M h264ref.CINT.PP200M 
            hmmer.CINT.PP200M lbm.CFP.PP200M leslie3d.CFP.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M milc.CFP.PP200M 
            namd.CFP.PP200M omnetpp.CINT.PP200M perlbench.CINT.PP200M povray.CFP.PP200M sjeng.CINT.PP200M soplex.CFP.PP200M
            sphinx3.CFP.PP200M tonto.CFP.PP200M wrf.CFP.PP200M xalancbmk.CINT.PP200M zeusmp.CFP.PP200M)
cd ${ROOT}
ORCS_D='C3'
mkdir ${ORCS_D}
# mkdir ${SINUCA_D}
# for i in ${LIST_BENCHS[@]}
# do  
for ((i=0;i<${#LIST_BENCHS[@]};i+=2))
do
#    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt "
    ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i]}'/'${LIST_BENCHS[i]} -f ${ORCS_D}'/'${LIST_BENCHS[i]}.txt&
    ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+1]}'/'${LIST_BENCHS[i+1]} -f ${ORCS_D}'/'${LIST_BENCHS[i+1]}.txt
#   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
#   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} >> /dev/null ; } 2>> ${ORCS_D}'/'${i}.txt & # salvar o tempo de execucao dos benchnmars
done
