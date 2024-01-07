#Chris Morrey 11/28/2023

import sys
sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import time
#from urllib import response
#from unicodedata import digit
#import numpy as np
#import pandas as pd
#from .things.modules import IO,eXarm,extruder
#from .things import define as D
from .things.modules import IO,eXarm,extruder
from.things import define as D
from .arm_sdk4.xarm.wrapper.xarm_api import XArmAPI as XArm


sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\things")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))


speaker = IO.initSpeech()
mod = IO.makeFrames(D.xlpath)[0]
move = IO.makeFrames(D.xlpath)[1]
speaker.Speak("File is loaded.")

#print ('arm status: error ',status[0],'warn ',status[1])
eXarm.armStart()
'''arm = XArmAPI(xArmIP,is_radian=False)
id = xArmSlaveID
baudRate = xArmBaudRate
arm.set_mode(0)
ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)
arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info forextruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
ret=[arm.warn_code,arm.error_code]
time.sleep(1)'''

speaker.Speak("Robot arm is connected.")

XArm.arm.reset(wait=True)
XArm.arm.get_err_warn_code(show=True, lang='en')
XArm.arm.clean_warn()
XArm.arm.clean_error()
XArm.arm.motion_enable(enable=True)
time.sleep(1)

IO.resetExtruder()

extruder.sender(D.init)
print('init packet sent')
speaker.Speak("Initializing")
dataRow = 0
while XArm.arm.connected:
    while dataRow <= mod.size / 11:
        print ('row: ',dataRow)
        XArm.arm.clean_error()
        XArm.arm.motion_enable(True)
        eXarm.setArmReady('TRUE')
        if (IO.getExtruderFlag('ready') == 1): 
            mv = IO.getIndex()
            datas = IO.getMod(mod,mv)   
            newMove = IO.getMove(move,mv)
            extruder.sender(datas[0])
            eXarm.mover(move)
            dataRow += 1
        elif (IO.getExtruderFlag('ready') == 0):
            time.sleep(.05)
IO.endAll(D.end,D.goHome)
