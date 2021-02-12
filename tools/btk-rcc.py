#!/usr/bin/env python3
#%%
import os,sys,json,argparse
#Btk resource complie

#Check file is exists
def fexists(f:str) -> bool:
    return os.access(f,os.F_OK)
#Process file
def process(fin,fout):
    pass
def trim_string(s : str) -> str:
    return s
def make_task(item:dict,res_file:str) -> dict:
    task = {}
    task['input'] = item['input']

    if item.get('output') == None:
        #It didnot provide the input file
        pass
    else:
        pass
    return task
def main(table:dict,res_file:str) -> None:
    print(table)
    for item in table:
        if item == 'comment':
            #skip comment
            continue
        #make task
        task = make_task(item,res_file)
        try:
            #open input
            fin  = open(task['input'],'rb')
            fout = open(task['output','wb'])
            process(fin,fout)
        except:
            fin.close()
            raise
    pass

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = "btk-rcc")

    parser.add_argument("-f",help="Target source file")

    args = parser.parse_args()
    print(args.values)