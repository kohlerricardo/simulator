#!/usr/bin/python3

import sys
from os import listdir
from os.path import isfile,isdir,join
import subprocess
import csv

def get_list_file(path):
    if isdir(path):
        onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]
    else:
        exit("Not a Dir")

    return onlyfiles
def get_data(path,files):
    dados=list()
    for file in files:
        data = subprocess.run(['grep','Instruction_Per_Cycle',path+file],stdout=subprocess.PIPE,universal_newlines=True)
        dados.append((data.args[2],data.stdout))
    out_file = list()
    for a in dados:    
        ipc = a[1].split('\n')
        ipc = " ".join(ipc)
        ipc = ipc.split(" ")
        # final = []
        # [final.append(x) for x in ipc if x not in final]
        out_file.append([a[0],ipc])
    return out_file
def sort_func(e):
    return e[0]
def main():
    fields_required=['Workload','Instruction_Per_Cycle'] 
    folder = sys.argv[1]
    files = get_list_file(folder)

    data_array = list()
    with open(sys.argv[2],'a') as csv_file:
        spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
        spaw_writter.writerow(fields_required)
    csv_file.close()

    data_array = get_data(folder,files)
    data_array.sort(key=sort_func)
    # print(data_array)

      
    for d in data_array:
        with open(sys.argv[2],'a') as csv_file:
            spaw_writter = csv.writer(csv_file,delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
            spaw_writter.writerow(d)



if __name__ == '__main__':
    if(len(sys.argv)!=3):
        exit("Use python parser_baseline.py <input folder> <output file>")
    main()