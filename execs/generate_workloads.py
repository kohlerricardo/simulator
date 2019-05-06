#!/usr/bin/python

import itertools
import random
# apps=['bwaves.CFP.PP200M', 'gcc.CINT.PP200M','libquantum.CINT.PP200M','mcf.CINT.PP200M','omnetpp.CINT.PP200M','soplex.CFP.PP200M','sphinx3.CFP.PP200M','wrf.CFP.PP200M']
# apps=['astar.CINT.PP200M', 'cactusADM.CFP.PP200M','bwaves.CFP.PP200M', 'gcc.CINT.PP200M','libquantum.CINT.PP200M','mcf.CINT.PP200M','omnetpp.CINT.PP200M','soplex.CFP.PP200M','sphinx3.CFP.PP200M','wrf.CFP.PP200M']
apps=['xalancbmk.CINT.PP200M', 'gromacs.CFP.PP200M','bwaves.CFP.PP200M', 'gcc.CINT.PP200M','libquantum.CINT.PP200M','mcf.CINT.PP200M','omnetpp.CINT.PP200M','soplex.CFP.PP200M','sphinx3.CFP.PP200M','wrf.CFP.PP200M']
tmp=[' '.join(lista) for lista in itertools.combinations(apps,4)]
workloads=list()
for a in tmp:
    split = a.split(" ")
    workloads.append(split)
selected=set()
for i in range(20):
    selected.add(random.randint(0,len(workloads)))
selected=list(selected)
for i in range(len(selected)):
    selected[i] = workloads[selected[i]]

for i in range(5):
    print selected[i]

workloads = list(itertools.chain.from_iterable(selected))
workloads = set(workloads)
print workloads