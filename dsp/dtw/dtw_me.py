from math import *  
import matplotlib.pyplot as plt  
import numpy  
import librosa
import librosa.display
import numpy as np
import matplotlib.pylab as plt
from dtw import dtw
"""
y1, sr1 = librosa.load('./down1.wav')
y2, sr2 = librosa.load('./down1.wav')

plt.subplot(1, 2, 1)
mfcc1 = librosa.feature.mfcc(y1, sr1)
librosa.display.specshow(mfcc1)

plt.subplot(1, 2, 2)
mfcc2 = librosa.feature.mfcc(y2, sr2)
librosa.display.specshow(mfcc2)
"""

def print_matrix(mat) :  
    print '[matrix] width : %d height : %d' % (len(mat[0]), len(mat))  
    print '-----------------------------------'  
    for i in range(len(mat)) :  
        print mat[i]#[v[:2] for v in mat[i]]  

def dist_for_float(p1, p2) :  
    dist = 0.0  
    elem_type = type(p1)  
    if  elem_type == float or elem_type == int :  
        dist = float(abs(p1 - p2))  
    else :  
        sumval = 0.0  
        for i in range(len(p1)) :  
            sumval += pow(p1[i] - p2[i], 2)  
        dist = pow(sumval, 0.5)  
    return dist  


def idtw(s1, s2, dist_func) :  
    w = len(s1)  
    h = len(s2)  

    mat = [([[0, 0, 0, 0] for j in range(w)]) for i in range(h)]  

    #print_matrix(mat)  

    for x in range(w) :  
        for y in range(h) :              
            dist = dist_func(s1[x], s2[y])  
            mat[y][x] = [dist, 0, 0, 0]  

    #print_matrix(mat)  

    elem_0_0 = mat[0][0]  
    elem_0_0[1] = elem_0_0[0] * 2  

    for x in range(1, w) :  
        mat[0][x][1] = mat[0][x][0] + mat[0][x - 1][1]  
        mat[0][x][2] = x - 1  
        mat[0][x][3] = 0  

    for y in range(1, h) :  
        mat[y][0][1] = mat[y][0][0] + mat[y - 1][0][1]              
        mat[y][0][2] = 0  
        mat[y][0][3] = y - 1  

    for y in range(1, h) :  
        for x in range(1, w) :  
            distlist = [mat[y][x - 1][1], mat[y - 1][x][1], 2 * mat[y - 1][x - 1][1]]  
            mindist = min(distlist)  
            idx = distlist.index(mindist)  
            mat[y][x][1] = mat[y][x][0] + mindist  
            if idx == 0 :  
                mat[y][x][2] = x - 1  
                mat[y][x][3] = y  
            elif idx == 1 :  
                mat[y][x][2] = x  
                mat[y][x][3] = y - 1  
            else :  
                mat[y][x][2] = x - 1  
                mat[y][x][3] = y - 1  

    result = mat[h - 1][w - 1]  
    retval = result[1]  
    path = [(w - 1, h - 1)]  
    while True :  
        x = result[2]  
        y = result[3]  
        path.append((x, y))  

        result = mat[y][x]  
        if x == 0 and y == 0 :  
            break  

    #print_matrix(mat)

    return retval, sorted(path)  

def display(s1, s2) :  

    val, path = dtw(s1, s2, dist_for_float)  

    w = len(s1)  
    h = len(s2)  

    mat = [[1] * w for i in range(h)]  
    for node in path :  
        x, y = node  
        mat[y][x] = 0  

    mat = numpy.array(mat)  

    plt.subplot(2, 2, 2)  
    c = plt.pcolor(mat, edgecolors='k', linewidths=4)  
    plt.title('Dynamic Time Warping (%f)' % val)  

    plt.subplot(2, 2, 1)  
    plt.plot(s2, range(len(s2)), 'g')  

    plt.subplot(2, 2, 4)  
    plt.plot(range(len(s1)), s1, 'r')  


    plt.show()  

from numpy.linalg import norm
np.set_printoptions(threshold='nan') 

testa = np.array([[ 12.76405235,  0.40015721,  0.97873798],
    [ 2.2408932 ,  2.2677152 , -0.57712067],
    [ 0.95008842,  0.79873121, -0.68033952],
    [ 0.4105985 ,  0.55464207,  0.77393398]])
#dist=lambda x, y: norm(x - y, ord=1)
testb = np.array([[ 1.76405235,  0.40015721,  1.97873798],
    [ 200.2408932 ,  2.2677152 , -0.57712067],
    [ 0.95008842,  0.79873121, -0.68033952],
    [ 3.4105985 ,  5.55464207,  0.77393398]])


s1 = [1, 2, 3, 4, 5, 5, 5, 4]  
s2 = np.array([[10, 1, 10, 5, 5, 4]])  
s3 = np.array([[1, 2, 3, 4, 5, 5]])  
s4 = [2, 3, 4, 5, 5, 5]  
val, path = idtw(s2, s3, dist_for_float)
print 'idtw distance between the two sounds:', val
#display(s1, s2)
dist, cost, acc_cost, path = dtw(s2, s3, dist=lambda x, y: norm(x - y, ord=1))
#dist, cost, acc_cost, path = dtw(s2, s3)
#val, path = dtw(s2, s3, dist=lambda x, y: norm(x - y, ord=1))
print 'dtw distance between the two sounds:', dist
