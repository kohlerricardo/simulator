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

    CONFS=(orcs_mshr_8_cas44 orcs_mshr_8_cas56 orcs_mshr_16_cas44 orcs_mshr_16_cas56 orcs_mshr_32_cas44 orcs_mshr_32_cas56)

    for i in ${CONFS[@]}
    do
        ln -s ../${i}/orcs .
        sleep 1
        echo "bash ../${i}/execs/exec_altix.sh ${i}_${1}"
        rm orcs
        sleep 1

    done
fi