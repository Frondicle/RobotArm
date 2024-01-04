#Chris Morrey 11/28/2023

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import sys
import time
from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import win32com.client 
from .things.modules import * 
from .things.define import *
from ..arm_sdk3.xarm.wrapper.xarm_api import XArmAPI

speaker = win32com.client.Dispatch("SAPI.SpVoice")
armStart()
speaker.Speak("Robot Arm Connected.")

arm.reset(wait=True)
arm.motion_enable(enable=True)
arm.get_err_warn_code(show=True, lang='en')
arm.clean_warn()
arm.clean_error()
time.sleep(1)

#initial packet:
init = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00]
sender(init)
print('init packet sent')
speaker.Speak("Initializing.")
line = 0
while arm.connected:
    while line <= mod.size / 11:
        print ('line: ',line)
        arm.clean_error()
        if (arm.get_tgpio_digital(0) == 1):    
            arm.set_state(state = 0)
            #print('ready flag ',arm.get_tgpio_digital(0))
            datas = modFrame(mod,line)   
            newMove = getMove(move,datas[1])
            arm.set_state(state=1)
            sender(datas[0])
            mover(newMove)
        line += 1
        #else:
            #time.sleep(.1)
#finish up:   
datas =[0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]
sender(datas)
mover([305,0,100,0,-180,0,xArmDefaultSpeed])#x=move[0], y=move[1], z=move[2], roll=move[4], pitch=move[3], yaw=move[5], speed=move[6]
time.sleep(2)
pd.ExcelFile.close(xlpath)
arm.reset(wait=True)
arm.disconnect()
