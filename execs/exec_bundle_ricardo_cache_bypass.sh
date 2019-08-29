#!/bin/bash

ROOT=`pwd`
EXEC="orcs"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"

DIR=Bundle_Ricardo_${1}/
RESULT_ALONE=Copy/
RESULT_WORKOAD=Random/
#lista aplicacoes utilizadas
APPS=(sphinx3.CFP.PP200M libquantum.CINT.PP200M wrf.CFP.PP200M mcf.CINT.PP200M omnetpp.CINT.PP200M soplex.CFP.PP200M bwaves.CFP.PP200M gcc.CINT.PP200M astar.CINT.PP200M xalancbmk.CINT.PP200M cactusADM.CFP.PP200M gromacs.CFP.PP200M)
# Lista de workloads

H0=(bwaves.CFP.PP200M gcc.CINT.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M)
H1=(libquantum.CINT.PP200M soplex.CFP.PP200M sphinx3.CFP.PP200M wrf.CFP.PP200M)
H2=(mcf.CINT.PP200M omnetpp.CINT.PP200M soplex.CFP.PP200M wrf.CFP.PP200M)
H3=(mcf.CINT.PP200M soplex.CFP.PP200M sphinx3.CFP.PP200M wrf.CFP.PP200M)
H4=(bwaves.CFP.PP200M gcc.CINT.PP200M mcf.CINT.PP200M soplex.CFP.PP200M)
H5=(gcc.CINT.PP200M libquantum.CINT.PP200M sphinx3.CFP.PP200M wrf.CFP.PP200M)
H6=(bwaves.CFP.PP200M gcc.CINT.PP200M omnetpp.CINT.PP200M sphinx3.CFP.PP200M)
H7=(gcc.CINT.PP200M libquantum.CINT.PP200M soplex.CFP.PP200M wrf.CFP.PP200M)
H8=(bwaves.CFP.PP200M gcc.CINT.PP200M soplex.CFP.PP200M sphinx3.CFP.PP200M)
H9=(bwaves.CFP.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M wrf.CFP.PP200M)


M0=(astar.CINT.PP200M cactusADM.CFP.PP200M bwaves.CFP.PP200M soplex.CFP.PP200M)
M1=(cactusADM.CFP.PP200M astar.CINT.PP200M libquantum.CINT.PP200M sphinx3.CFP.PP200M)
M2=(mcf.CINT.PP200M astar.CINT.PP200M cactusADM.CFP.PP200M sphinx3.CFP.PP200M)
M3=(astar.CINT.PP200M cactusADM.CFP.PP200M gcc.CINT.PP200M soplex.CFP.PP200M)
M4=(cactusADM.CFP.PP200M bwaves.CFP.PP200M omnetpp.CINT.PP200M astar.CINT.PP200M)


L0=(xalancbmk.CINT.PP200M gromacs.CFP.PP200M bwaves.CFP.PP200M soplex.CFP.PP200M)
L1=(gromacs.CFP.PP200M bwaves.CFP.PP200M omnetpp.CINT.PP200M xalancbmk.CINT.PP200M)
L2=(soplex.CFP.PP200M xalancbmk.CINT.PP200M gromacs.CFP.PP200M sphinx3.CFP.PP200M)
L3=(xalancbmk.CINT.PP200M libquantum.CINT.PP200M mcf.CINT.PP200M gromacs.CFP.PP200M)
L4=(gromacs.CFP.PP200M xalancbmk.CINT.PP200M soplex.CFP.PP200M wrf.CFP.PP200M)

LOCATION=${TRACE_FOLDER}${BENCHMARK_FOLDER}
COMANDO=${ROOT}/./${EXEC}

mkdir -p ${DIR}${RESULT_ALONE}
mkdir -p ${DIR}${RESULT_WORKOAD}
# Executandoapps standalone
for APP in ${APPS[@]}
do 
   byobu new-window ${COMANDO} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -f ${DIR}${RESULT_ALONE}${APP}.txt
done

#Executando simulacao workloads
byobu new-window ${COMANDO} -t ${LOCATION}${H0[0]}/${H0[0]} -t ${LOCATION}${H0[1]}/${H0[1]} -t ${LOCATION}${H0[2]}/${H0[2]} -t ${LOCATION}${H0[3]}/${H0[3]} -f ${DIR}${RESULT_WORKOAD}H0.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H1[0]}/${H1[0]} -t ${LOCATION}${H1[1]}/${H1[1]} -t ${LOCATION}${H1[2]}/${H1[2]} -t ${LOCATION}${H1[3]}/${H1[3]} -f ${DIR}${RESULT_WORKOAD}H1.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H2[0]}/${H2[0]} -t ${LOCATION}${H2[1]}/${H2[1]} -t ${LOCATION}${H2[2]}/${H2[2]} -t ${LOCATION}${H2[3]}/${H2[3]} -f ${DIR}${RESULT_WORKOAD}H2.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H3[0]}/${H3[0]} -t ${LOCATION}${H3[1]}/${H3[1]} -t ${LOCATION}${H3[2]}/${H3[2]} -t ${LOCATION}${H3[3]}/${H3[3]} -f ${DIR}${RESULT_WORKOAD}H3.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H4[0]}/${H4[0]} -t ${LOCATION}${H4[1]}/${H4[1]} -t ${LOCATION}${H4[2]}/${H4[2]} -t ${LOCATION}${H4[3]}/${H4[3]} -f ${DIR}${RESULT_WORKOAD}H4.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H5[0]}/${H5[0]} -t ${LOCATION}${H5[1]}/${H5[1]} -t ${LOCATION}${H5[2]}/${H5[2]} -t ${LOCATION}${H5[3]}/${H5[3]} -f ${DIR}${RESULT_WORKOAD}H5.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H6[0]}/${H6[0]} -t ${LOCATION}${H6[1]}/${H6[1]} -t ${LOCATION}${H6[2]}/${H6[2]} -t ${LOCATION}${H6[3]}/${H6[3]} -f ${DIR}${RESULT_WORKOAD}H6.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H7[0]}/${H7[0]} -t ${LOCATION}${H7[1]}/${H7[1]} -t ${LOCATION}${H7[2]}/${H7[2]} -t ${LOCATION}${H7[3]}/${H7[3]} -f ${DIR}${RESULT_WORKOAD}H7.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H8[0]}/${H8[0]} -t ${LOCATION}${H8[1]}/${H8[1]} -t ${LOCATION}${H8[2]}/${H8[2]} -t ${LOCATION}${H8[3]}/${H8[3]} -f ${DIR}${RESULT_WORKOAD}H8.txt
byobu new-window ${COMANDO} -t ${LOCATION}${H9[0]}/${H9[0]} -t ${LOCATION}${H9[1]}/${H9[1]} -t ${LOCATION}${H9[2]}/${H9[2]} -t ${LOCATION}${H9[3]}/${H9[3]} -f ${DIR}${RESULT_WORKOAD}H9.txt
# Workload media intensidade
byobu new-window ${COMANDO} -t ${LOCATION}${M0[0]}/${M0[0]} -t ${LOCATION}${M0[1]}/${M0[1]} -t ${LOCATION}${M0[2]}/${M0[2]} -t ${LOCATION}${M0[3]}/${M0[3]} -f ${DIR}${RESULT_WORKOAD}M0.txt
byobu new-window ${COMANDO} -t ${LOCATION}${M1[0]}/${M1[0]} -t ${LOCATION}${M1[1]}/${M1[1]} -t ${LOCATION}${M1[2]}/${M1[2]} -t ${LOCATION}${M1[3]}/${M1[3]} -f ${DIR}${RESULT_WORKOAD}M1.txt
byobu new-window ${COMANDO} -t ${LOCATION}${M2[0]}/${M2[0]} -t ${LOCATION}${M2[1]}/${M2[1]} -t ${LOCATION}${M2[2]}/${M2[2]} -t ${LOCATION}${M2[3]}/${M2[3]} -f ${DIR}${RESULT_WORKOAD}M2.txt
byobu new-window ${COMANDO} -t ${LOCATION}${M3[0]}/${M3[0]} -t ${LOCATION}${M3[1]}/${M3[1]} -t ${LOCATION}${M3[2]}/${M3[2]} -t ${LOCATION}${M3[3]}/${M3[3]} -f ${DIR}${RESULT_WORKOAD}M3.txt
byobu new-window ${COMANDO} -t ${LOCATION}${M4[0]}/${M4[0]} -t ${LOCATION}${M4[1]}/${M4[1]} -t ${LOCATION}${M4[2]}/${M4[2]} -t ${LOCATION}${M4[3]}/${M4[3]} -f ${DIR}${RESULT_WORKOAD}M4.txt
# Workload media intensidade
byobu new-window ${COMANDO} -t ${LOCATION}${L0[0]}/${L0[0]} -t ${LOCATION}${L0[1]}/${L0[1]} -t ${LOCATION}${L0[2]}/${L0[2]} -t ${LOCATION}${L0[3]}/${L0[3]} -f ${DIR}${RESULT_WORKOAD}L0.txt
byobu new-window ${COMANDO} -t ${LOCATION}${L1[0]}/${L1[0]} -t ${LOCATION}${L1[1]}/${L1[1]} -t ${LOCATION}${L1[2]}/${L1[2]} -t ${LOCATION}${L1[3]}/${L1[3]} -f ${DIR}${RESULT_WORKOAD}L1.txt
byobu new-window ${COMANDO} -t ${LOCATION}${L2[0]}/${L2[0]} -t ${LOCATION}${L2[1]}/${L2[1]} -t ${LOCATION}${L2[2]}/${L2[2]} -t ${LOCATION}${L2[3]}/${L2[3]} -f ${DIR}${RESULT_WORKOAD}L2.txt
byobu new-window ${COMANDO} -t ${LOCATION}${L3[0]}/${L3[0]} -t ${LOCATION}${L3[1]}/${L3[1]} -t ${LOCATION}${L3[2]}/${L3[2]} -t ${LOCATION}${L3[3]}/${L3[3]} -f ${DIR}${RESULT_WORKOAD}L3.txt
byobu new-window ${COMANDO} -t ${LOCATION}${L4[0]}/${L4[0]} -t ${LOCATION}${L4[1]}/${L4[1]} -t ${LOCATION}${L4[2]}/${L4[2]} -t ${LOCATION}${L4[3]}/${L4[3]} -f ${DIR}${RESULT_WORKOAD}L4.txt
