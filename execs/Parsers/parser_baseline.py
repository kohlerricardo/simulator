#!/usr/bin/python

import sys
from os import listdir
from os.path import isfile,isdir,join
import csv

def get_list_file(path):
    if isdir(path):
        onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]
    else:
        exit("Not a Dir")

    return onlyfiles
def read_file(path):
    fields = list()
    with(open(path))as file_open:
        for line in file_open:
            if '#' in line:
                pass
            elif '=' in line:
                pass
            else:
                fields.append(line)
    return fields
def generate_dict(dados):
    dicionario = dict()
    for i in dados:
        i = i.replace('\n','')
        i = i.split(":")
        if len(i)>1:
            dicionario[i[0]]=i[1]
    return dicionario
def get_values(dados,dicionario):
    values=[]
    for i in dados:
        values.append(dicionario.get(i))
    return values
def main():
    fields_required=['benchmark','Total_Cycle','fetch_instructions','INST_CACHE_Cache_Hits','INST_CACHE_Cache_Miss','L1_DATA_CACHE_Cache_Hits','L1_DATA_CACHE_Cache_Miss','LLC_Cache_Hits','LLC_Cache_Miss','EMC_DATA_CACHE_Cache_Hits','EMC_DATA_CACHE_Cache_Miss','EMC_Access_LLC_HIT','EMC_Access_LLC_MISS','times_llc_rob_head','numero_load_deps','Total_instrucoes_dependentes','canceled_emc_execution'] 
    folder = sys.argv[1]
    files = get_list_file(folder)
    with open(sys.argv[2],'a') as csv_file:
        spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
        spaw_writter.writerow(fields_required)
    csv_file.close()
    for f in files:
        dados = read_file(folder+f)
        dicionario = generate_dict(dados)
        dicionario['benchmark'] = f
        with open(sys.argv[2],'a') as csv_file:
            spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
            spaw_writter.writerow(get_values(fields_required,dicionario))
        csv_file.close()



if __name__ == '__main__':
    if(len(sys.argv)!=3):
        exit("Use python parser_baseline.py <input folder> <output file>")
    main()