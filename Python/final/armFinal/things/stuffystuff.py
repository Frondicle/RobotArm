#Chris Morrey 11/28/2023

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import sys
import time
from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import win32com.client 
from .modules import *
from ..arm_sdk3.xarm.wrapper.xarm_api import XArmAPI

speaker = win32com.client.Dispatch("SAPI.SpVoice")

arm = XArmAPI('192.168.1.240',is_radian=False)
id = 7
baudRate = 9600
code, digitals = arm.get_tgpio_digital()
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)

arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)

speaker.Speak("Robot Arm Connected.")

armspeed = 100

arm.reset(wait=True)
arm.motion_enable(enable=True)
arm.get_err_warn_code(show=True, lang='en')
arm.clean_warn()
arm.clean_error()
time.sleep(1)
'''Format from Python: [0x09=slave id,0x10=function 16 write registers
0x00,0x00=beginning address to write,0x00,0x06 = number of registers
0x0c=number of bytes(2x registers)
0x00,0x100=register 0 index
0x00,0x00=register 1 steps
0x00,0x00=register 2 direction(0 for backward, 1 for forward)
0x00,0x00=register 3 speed (in steps per second)
0x00,0x00=register 4 end of file(0 for no, 1 for yes)
0x00,0x00=register 5 index out '''
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
mover([305,0,100,0,-180,0,armspeed])#x=move[0], y=move[1], z=move[2], roll=move[4], pitch=move[3], yaw=move[5], speed=move[6]
time.sleep(2)
pd.ExcelFile.close(xlpath)
arm.reset(wait=True)
arm.disconnect()
