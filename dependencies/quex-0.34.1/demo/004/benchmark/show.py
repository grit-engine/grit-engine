#! /usr/bin/env python
import sys
import os


dir = "./results/"
file_list = os.listdir(dir)

db = []
for file in file_list:
    data = []
    if file[0] == ".": continue

    for line in open(dir + file, "r").readlines():
        x = line.split()
        if line.find("quex_version") != -1:               version = x[2][1:-2]
        if line.find("cpu_name") != -1:                   cpu = x[2][1:-2]
        if line.find("clock_cycles_per_character") != -1: ccc = x[2][1:-2]
        # if line.find("file_name") != -1: test_file = x[2][1:-2]
       
        if line.strip() == "}": data.append(ccc)

    db.append([version, cpu] + data)
        
db.sort()
for entry in db:
    print repr(entry)[1:-1]
