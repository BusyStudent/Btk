#!/usr/bin/env python3
import os,sys,json
#Btk resource complie


#Process file
def process(fin,fout):
    pass
def trim_string(s):
    return s
def make_task(item,res_file):
    task = {}
    task['input'] = item['input']

    if item.get('output') == None:
        #It didnot provide the input file
        pass
    else:
        pass
    return task
def main(table,res_file):
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
    if len(sys.argv) <= 1:
        print('useage btk-rcc [resource_list.json]')
    else:
        res_file = sys.argv[0]

        main(json.load(open(res_file)),res_file)