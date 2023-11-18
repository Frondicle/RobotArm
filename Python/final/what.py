import sys
import time
import numpy as np
import pandas as pd
sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")

mod = pd.read_excel("C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\grassData.xlsx", "1", index_col='line',usecols=['line','move','espeed','dir','end'],converters={'line':hex,'move':hex,'espeed':hex,'dir':hex,'end':hex})

move = pd.read_excel("C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\grassData.xlsx", "1", index_col='line',usecols=['line','x','y','z','pitch','roll','yaw','fspeed'])
ln=1
while ln < 10:
    def getMove(frame,z):
        move = frame.loc(line=z) 
        ptx = move('x')
        pty = move('y')
        ptz = move('z')
        ptp = move('pitch')
        ptr = move('roll')
        ptyaw = move('yaw')
        pts = move('fspeed')
        mv = [ptx,pty,ptz,ptp,ptr,ptyaw,pts]
        print (move)
        print (mv,sep=',')
        return mv

    def modFrame(frame,z):
        modData=frame.loc(line=z)
        movement=modData('move')
        spd=modData('espeed')
        dir=modData('dir')
        end=modData('end')
        dframe = ('0x09,0x10,0x00,0x00,0x00,0x04,0x08,',movement,',',spd,',',dir,',',end)
        print(dframe)
        print(modData)
        return dframe
getMove(move,ln)
modFrame(mod,ln)    
ln=ln+1