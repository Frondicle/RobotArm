#generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math
#import sender, buildFrame, errors

data = open('C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\programs\\ext_data.txt','r', buffering=1)
moves = 0

def hexconvert(x,big):
    global moves
    z = (x * 100)
    print(z)
    h = str(hex(z))
    s = h.lstrip("0x")
    e = s.zfill(4)
    if big:
        f = e[0]
        g = e[1]
        h = e[2]
        i = e[3]
    else:
        f = e[2]
        g = e[3]
        h = e[0]
        i = e[1]
    j = "0x"
    k = ","
    a = j+f+g+k+j+h+i
    b=z
    return a,b

def buildFrame(filedata):
    a = filedata.readline()
    s = a.rstrip("\n")
    b = s.split(',')
    print(b)
    for x in b:
        b[x] = int(b[x])
    frame = ('0x09,0x10,0x00,0x00,0x00,0x03,0x06,',hexconvert(b[0],True),hexconvert(b[1],True),hexconvert(b[2],True))
    print(frame)
    time.sleep(.25)
buildFrame(data)
