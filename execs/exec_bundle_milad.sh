#!/bin/bash

ROOT=`pwd`
EXEC="orcs"
TRACE_FOLDER="traces/"
BENCHMARK_FOLDER="spec_cpu2006/"

DIR=Bundle_Milad/
# RESULT_ALONE=Copy/
RESULT_WORKOAD=${1}/
#lista aplicacoes utilizadas
APPS=(libquantum.CINT.PP200M mcf.CINT.PP200M lbm.CFP.PP200M milc.CFP.PP200M soplex.CFP.PP200M sphinx3.CFP.PP200M bwaves.CFP.PP200M omnetpp.CINT.PP200M)
# Lista de workloads
W0=(bwaves.CFP.PP200M lbm.CFP.PP200M milc.CFP.PP200M omnetpp.CINT.PP200M)
W1=(soplex.CFP.PP200M omnetpp.CINT.PP200M bwaves.CFP.PP200M libquantum.CINT.PP200M)
W2=(sphinx3.CFP.PP200M mcf.CINT.PP200M omnetpp.CINT.PP200M milc.CFP.PP200M)
W3=(mcf.CINT.PP200M sphinx3.CFP.PP200M soplex.CFP.PP200M libquantum.CINT.PP200M)
W4=(lbm.CFP.PP200M mcf.CINT.PP200M libquantum.CINT.PP200M bwaves.CFP.PP200M)
W5=(lbm.CFP.PP200M soplex.CFP.PP200M mcf.CINT.PP200M milc.CFP.PP200M)
W6=(bwaves.CFP.PP200M libquantum.CINT.PP200M sphinx3.CFP.PP200M omnetpp.CINT.PP200M)
W7=(omnetpp.CINT.PP200M soplex.CFP.PP200M mcf.CINT.PP200M bwaves.CFP.PP200M)
W8=(lbm.CFP.PP200M mcf.CINT.PP200M libquantum.CINT.PP200M soplex.CFP.PP200M)
W9=(libquantum.CINT.PP200M bwaves.CFP.PP200M soplex.CFP.PP200M omnetpp.CINT.PP200M)
LOCATION=${TRACE_FOLDER}${BENCHMARK_FOLDER}
COMANDO=${ROOT}/./${EXEC}

# mkdir -p ${DIR}${RESULT_ALONE}
mkdir -p ${DIR}${RESULT_WORKOAD}
# Executandoapps standalone
# for APP in ${APPS[@]}
# do 
# nohup byobu new-window ${COMANDO} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -t ${LOCATION}${APP}/${APP} -f ${DIR}${RESULT_ALONE}${APP}.txt&
# done

#Executando simulacao workloads
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W0[0]}/${W0[0]} -t ${LOCATION}${W0[1]}/${W0[1]} -t ${LOCATION}${W0[2]}/${W0[2]} -t ${LOCATION}${W0[3]}/${W0[3]} -f ${DIR}${RESULT_WORKOAD}W0.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W1[0]}/${W1[0]} -t ${LOCATION}${W1[1]}/${W1[1]} -t ${LOCATION}${W1[2]}/${W1[2]} -t ${LOCATION}${W1[3]}/${W1[3]} -f ${DIR}${RESULT_WORKOAD}W1.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W2[0]}/${W2[0]} -t ${LOCATION}${W2[1]}/${W2[1]} -t ${LOCATION}${W2[2]}/${W2[2]} -t ${LOCATION}${W2[3]}/${W2[3]} -f ${DIR}${RESULT_WORKOAD}W2.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W3[0]}/${W3[0]} -t ${LOCATION}${W3[1]}/${W3[1]} -t ${LOCATION}${W3[2]}/${W3[2]} -t ${LOCATION}${W3[3]}/${W3[3]} -f ${DIR}${RESULT_WORKOAD}W3.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W4[0]}/${W4[0]} -t ${LOCATION}${W4[1]}/${W4[1]} -t ${LOCATION}${W4[2]}/${W4[2]} -t ${LOCATION}${W4[3]}/${W4[3]} -f ${DIR}${RESULT_WORKOAD}W4.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W5[0]}/${W5[0]} -t ${LOCATION}${W5[1]}/${W5[1]} -t ${LOCATION}${W5[2]}/${W5[2]} -t ${LOCATION}${W5[3]}/${W5[3]} -f ${DIR}${RESULT_WORKOAD}W5.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W6[0]}/${W6[0]} -t ${LOCATION}${W6[1]}/${W6[1]} -t ${LOCATION}${W6[2]}/${W6[2]} -t ${LOCATION}${W6[3]}/${W6[3]} -f ${DIR}${RESULT_WORKOAD}W6.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W7[0]}/${W7[0]} -t ${LOCATION}${W7[1]}/${W7[1]} -t ${LOCATION}${W7[2]}/${W7[2]} -t ${LOCATION}${W7[3]}/${W7[3]} -f ${DIR}${RESULT_WORKOAD}W7.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W8[0]}/${W8[0]} -t ${LOCATION}${W8[1]}/${W8[1]} -t ${LOCATION}${W8[2]}/${W8[2]} -t ${LOCATION}${W8[3]}/${W8[3]} -f ${DIR}${RESULT_WORKOAD}W8.txt&
nohup byobu new-window ${COMANDO} -t ${LOCATION}${W9[0]}/${W9[0]} -t ${LOCATION}${W9[1]}/${W9[1]} -t ${LOCATION}${W9[2]}/${W9[2]} -t ${LOCATION}${W9[3]}/${W9[3]} -f ${DIR}${RESULT_WORKOAD}W9.txt&
