import numpy as np
"""
f = open(r"no1a.txt")
line = f.read.splitlines()
data_list = []
while line:
    num = list(map(int,line.split()))
    data_list.append(num)
    line = f.readline()
f.close()
data_array = np.array(data_list)
with open("no1.txt", "r") as f:
        data = f.read().splitlines()
print(data)
"""
arr = np.loadtxt('no1.txt', skiprows=1)
print arr
