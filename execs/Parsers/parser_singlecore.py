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
        if i is None:
            values.append("")
        else: 
            values.append(dicionario.get(i))
    return values
def main():
    # fields_required=['benchmark','Total_Cycle','fetch_instructions',None,'Stage_Rename','INST_CACHE_Cache_Hits','INST_CACHE_Cache_Miss',None,'L1_DATA_CACHE_Cache_Hits','L1_DATA_CACHE_Cache_Miss',None,'LLC_Cache_Hits','LLC_Cache_Miss',None,'times_llc_rob_head','started_emc_execution','canceled_emc_execution'] 
    fields_required=['benchmark','Total_Cycle','fetch_instructions',None,None,'times_llc_rob_head','started_emc_execution','LLC_Cache_Miss','numero_load_deps','oracle_llc_miss','total_instruction_sent_emc','loads_missed_counter','started_emc_without_loads','EMC_Access_LLC_MISS','EMC_Correct_Prediction_RAM'] 
    folder = sys.argv[1]
    files = get_list_file(folder)
    data_array = list()
    with open(sys.argv[2],'a') as csv_file:
        spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
        spaw_writter.writerow(fields_required)
    csv_file.close()
    for f in files:
        dados = read_file(folder+f)
        dicionario = generate_dict(dados)
        dicionario['benchmark'] = str(f).lower()
        data_array.append(get_values(fields_required,dicionario))
        data_array = sorted(data_array)
    for d in data_array:
        # print d
        # d.reverse()
        # d[1]=str(int(d[0])+int(d[1]))
        # d[2]=str(int(d[2])-int(d[0]))
        # del d[0]
        # d.reverse()
        # print d
        with open(sys.argv[2],'a') as csv_file:
            spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
            spaw_writter.writerow(d)



if __name__ == '__main__':
    if(len(sys.argv)!=3):
        exit("Use python parser_baseline.py <input folder> <output file>")
    main()