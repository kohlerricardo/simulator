#!/bin/bash

ROOT=`pwd`
EXEC="orcs"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"
LIST_BENCHS=(astar.CINT.PP200M bwaves.CFP.PP200M bzip2.CINT.PP200M cactusADM.CFP.PP200M calculix.CFP.PP200M dealII.CFP.PP200M 
            gamess.CFP.PP200M gcc.CINT.PP200M GemsFDTD.CFP.PP200M gobmk.CINT.PP200M gromacs.CFP.PP200M h264ref.CINT.PP200M 
            hmmer.CINT.PP200M lbm.CFP.PP200M leslie3d.CFP.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M milc.CFP.PP200M 
            namd.CFP.PP200M omnetpp.CINT.PP200M perlbench.CINT.PP200M povray.CFP.PP200M sjeng.CINT.PP200M soplex.CFP.PP200M
            sphinx3.CFP.PP200M tonto.CFP.PP200M wrf.CFP.PP200M xalancbmk.CINT.PP200M zeusmp.CFP.PP200M)

H1="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[13]}/${LIST_BENCHS[13]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[17]}/${LIST_BENCHS[17]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -f H1.txt"
H2="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[15]}/${LIST_BENCHS[15]} -f H2.txt"
H3="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[24]}/${LIST_BENCHS[24]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[17]}/${LIST_BENCHS[17]} -f H3.txt"
H4="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[24]}/${LIST_BENCHS[24]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[17]}/${LIST_BENCHS[17]} -f H4.txt"
H5="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[13]}/${LIST_BENCHS[13]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[15]}/${LIST_BENCHS[15]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -f H5.txt"
H6="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[13]}/${LIST_BENCHS[13]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[17]}/${LIST_BENCHS[17]} -f H6.txt"
H7="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[15]}/${LIST_BENCHS[15]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[24]}/${LIST_BENCHS[24]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -f H7.txt"
H8="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -f H8.txt"
H9="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[13]}/${LIST_BENCHS[13]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[16]}/${LIST_BENCHS[16]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[15]}/${LIST_BENCHS[15]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -f H9.txt"
H10="-t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[15]}/${LIST_BENCHS[15]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[1]}/${LIST_BENCHS[1]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[23]}/${LIST_BENCHS[23]} -t ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[19]}/${LIST_BENCHS[19]} -f H10.txt"
WORKLOADS=(H1 H2 H3 H4 H5 H6 H7 H8 H9 H10)
        echo byobu new-window "$EXEC ${H1}"
        echo byobu new-window "$EXEC ${H2}"
        echo byobu new-window "$EXEC ${H3}"
        echo byobu new-window "$EXEC ${H4}"
        echo byobu new-window "$EXEC ${H5}"
        echo byobu new-window "$EXEC ${H6}"
        echo byobu new-window "$EXEC ${H7}"
        echo byobu new-window "$EXEC ${H8}"
        echo byobu new-window "$EXEC ${H9}"
        echo byobu new-window "$EXEC ${H10}"
=


#    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} -f ${ORCS_D}'/'${i}.txt" "
    # ${ROOT}'/'.${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i]}'/'${LIST_BENCHS[i]} -f ${ORCS_D}'/'${LIST_BENCHS[i]}.txt"&
    # ${ROOT}'/'.${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${LIST_BENCHS[i+1]}'/'${LIST_BENCHS[i+1]} -f ${ORCS_D}'/'${LIST_BENCHS[i+1]}.txt"
