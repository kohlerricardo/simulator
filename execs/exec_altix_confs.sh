#!/bin/bash

ROOT='/home/ricardo'
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
cd ${ROOT}'/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}
LIST_BENCHS=(astar.CINT.PP200M bwaves.CFP.PP200M bzip2.CINT.PP200M cactusADM.CFP.PP200M calculix.CFP.PP200M dealII.CFP.PP200M gamess.CFP.PP200M 
                gcc.CINT.PP200M GemsFDTD.CFP.PP200M gobmk.CINT.PP200M gromacs.CFP.PP200M h264ref.CINT.PP200M hmmer.CINT.PP200M lbm.CFP.PP200M 
                leslie3d.CFP.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M milc.CFP.PP200M namd.CFP.PP200M omnetpp.CINT.PP200M perlbench.CINT.PP200M 
                povray.CFP.PP200M sjeng.CINT.PP200M soplex.CFP.PP200M sphinx3.CFP.PP200M tonto.CFP.PP200M wrf.CFP.PP200M xalancbmk.CINT.PP200M 
                zeusmp.CFP.PP200M)

cd ${ROOT}
ORCS_D=(all_op_rh.d)
mkdir 'all_op_rh.d'
# mkdir ${SINUCA_D}
# for i in ${ORCS_D[@]}
# do
# cd /home/ricardo/${i}
# make clean && make -j 4 all
# done
# cd -
      
        for ((i=0;i<${#LIST_BENCHS[@]};i+=3))
        do  
        # mkdir ${i}'.d'
        # echo "${ROOT}'/'${jj}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${jj}.d'/'${i}.txt "
        ${ROOT}'/'${ORCS_D[0]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i]}'/'${LIST_BENCHS[i]} -f '/home/ricardo/bin/'${ORCS_D[0]}'/'${LIST_BENCHS[i]}.txt& 
        ${ROOT}'/'${ORCS_D[0]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+1]}'/'${LIST_BENCHS[i+1]} -f '/home/ricardo/bin/'${ORCS_D[0]}'/'${LIST_BENCHS[i+1]}.txt&
        ${ROOT}'/'${ORCS_D[0]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+2]}'/'${LIST_BENCHS[i+2]} -f '/home/ricardo/bin/'${ORCS_D[0]}'/'${LIST_BENCHS[i+2]}.txt
        # echo ${ROOT}'/'${ORCS_D[0]}'/'./${EXEC} '/home/ricardo/bin/'${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+3]}'/'${LIST_BENCHS[i+3]} -f '/home/ricardo/bin/'${ORCS_D[0]}'/'${LIST_BENCHS[i+3]}.txt 
        #   { time ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt ; } 2>> ${ORCS_D}'/'${i}.txt &
        done