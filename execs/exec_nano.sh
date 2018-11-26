#!/bin/bash

ROOT=`pwd`
EXEC="orcs -t"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="nanobench/"

NANO_D="nano.d"
mkdir ${NANO_D}

benchmark_control(){
BENCHMARK_CONTROL=(control_complex.CONTROL control_conditional.CONTROL 
                    control_random.CONTROL control_small_bbl.CONTROL 
                    control_switch.CONTROL)
for i in ${BENCHMARK_CONTROL[@]}
do  
    # echo ${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
benchmark_dependency(){
BENCHMARK_DEPENDENCY=(dependency_chain1.DEPENDENCY dependency_chain2.DEPENDENCY
                    dependency_chain3.DEPENDENCY dependency_chain4.DEPENDENCY
                    dependency_chain5.DEPENDENCY dependency_chain6.DEPENDENCY)
for i in ${BENCHMARK_DEPENDENCY[@]}
do  
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
benchmark_execution(){
BENCHMARK_EXECUTION=(fp_add.EXECUTION fp_div.EXECUTION fp_mul.EXECUTION 
                    int_add.EXECUTION int_div.EXECUTION int_mul.EXECUTION)
for i in ${BENCHMARK_EXECUTION[@]}
do  
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
benchmark_ld_mem_dep(){
BENCHMARK_LD_MEM_DEP=(load_dep_00016kb.MEMORY load_dep_00032kb.MEMORY load_dep_00064kb.MEMORY
                    load_dep_00128kb.MEMORY load_dep_00256kb.MEMORY load_dep_00512kb.MEMORY
                    load_dep_01024kb.MEMORY load_dep_02048kb.MEMORY load_dep_04096kb.MEMORY 
                    load_dep_08192kb.MEMORY load_dep_16384kb.MEMORY load_dep_32768kb.MEMORY)
for i in ${BENCHMARK_LD_MEM_DEP[@]}
do  
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
benchmark_ld_mem_indep(){
BENCHMARK_LD_MEM_INDEP=(load_ind_00016kb.MEMORY load_ind_00032kb.MEMORY load_ind_00064kb.MEMORY
                        load_ind_00128kb.MEMORY load_ind_00256kb.MEMORY load_ind_00512kb.MEMORY
                        load_ind_01024kb.MEMORY load_ind_02048kb.MEMORY load_ind_04096kb.MEMORY
                        load_ind_08192kb.MEMORY load_ind_16384kb.MEMORY load_ind_32768kb.MEMORY)
for i in ${BENCHMARK_LD_MEM_INDEP[@]}
do  
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
benchmark_st_mem_indep(){                        
BENCHMARK_ST_MEM_INDEP=(store_ind_00016kb.MEMORY store_ind_00032kb.MEMORY store_ind_00064kb.MEMORY
                        store_ind_00128kb.MEMORY store_ind_00256kb.MEMORY store_ind_00512kb.MEMORY
                        store_ind_01024kb.MEMORY store_ind_02048kb.MEMORY store_ind_04096kb.MEMORY
                        store_ind_08192kb.MEMORY store_ind_16384kb.MEMORY store_ind_32768kb.MEMORY )
for i in ${BENCHMARK_ST_MEM_INDEP[@]}
do  
    byobu new-window "${ROOT}'/'./${EXEC} ${TRACE_FOLDER}${BENCHMARK_FOLDER}${i}'/'${i} "-f" ${NANO_D}'/'${i}.txt"
    # sleep 5
done
}
all(){
    benchmark_control
    benchmark_dependency
    benchmark_execution
    benchmark_ld_mem_dep
    benchmark_ld_mem_indep
    benchmark_st_mem_indep
}
menu(){
echo "****************************************************************"
echo "****************************************************************"
echo "*****              Execução NanoBenchmarks                ******"
echo "*****                                                     ******"
echo "****************************************************************"
echo "****************************************************************"
echo "1) Benchmarks Controle"
echo "2) Benchmarks Dependencias"
echo "3) Benchmarks Execução"
echo "4) Benchmarks Loads Dependentes"
echo "5) Benchmarks Loads Independentes"
echo "6) Benchmarks Stores Independentes"
echo "****************************************************************"
echo "****************************************************************"
}
    menu
    read op
    case ${op} in
        1)
        benchmark_control
        ;;
        2)
        benchmark_dependency
        ;;
        3)
        benchmark_execution
        ;;
        4)
        benchmark_ld_mem_dep
        ;;
        5)
        benchmark_ld_mem_indep
        ;;
        6)
        benchmark_st_mem_indep
        ;;
        *)
        all
        ;;
    esac