#Chris Morrey 1/25/2023
#modbus and buffer only
import sys
import time
#from urllib import response
#from unicodedata import digit
#import numpy as np
import pandas as pd
import numpy as np
import win32com.client
import logging 
logging.basicConfig(filename='output.txt',level=logging.DEBUG)

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\arm_sdk4")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\grassDataWalls.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['line','move','espeed','dir','end'])

ip = '192.168.1.240'
arm = XArmAPI(ip)

speaker = win32com.client.Dispatch("SAPI.SpVoice")

def getMod(frame,ln):
    modData=frame.iloc[ln]
    indx = modData['index']
    move=modData['move']
    spd=modData['espeed']
    dir=modData['dir']
    end=modData['end']
    dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx, move, spd, dir, end]
    '''dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx // 256 % 256, indx // 256, move // 256 % 256, move // 256, spd // 256 % 256, spd // 256, dir // 256 % 256, dir // 256, end // 256 % 256,end // 256]'''
    response = [dframe,indx]
    return response

def sender(data_frame):
    ret = arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    if ret[0] == 0:
        print ('sender: data_frame:',data_frame)
        print('sender: modresponse: ',ret[1][0],ret[1][1],ret[1][2],ret[1][3],ret[1][4],ret[1][5],ret[1][6],ret[1][7],ret[1][8],ret[1][9],ret[1][10],ret[1][11],ret[1][12])
    else:
        print('sender: ',ret[0])
    arm.clean_error()

def resetExtruder():
    arm.set_tgpio_digital(0,1)
    time.sleep(1)
    arm.set_tgpio_digital(0,0)

def endAll(): #changed from original!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    time.sleep(2)
    speaker.speak("Shutting down extruder.")
    resetExtruder()
    #pd.ExcelFile.close(xlpath)
    arm.reset(wait=True)
    arm.set_state(state=4)
    arm.disconnect()

speaker.speak("Resetting Extruder.")
resetExtruder()
time.sleep(3)

arm.motion_enable(True)
arm.clean_error()
arm.set_mode(0)
arm.set_state(state = 0)
arm.reset(wait=True)
time.sleep(1)

id = 7
baudRate = 9600
ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]),'line 205')
arm.clean_error()    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)
arm.set_state(state=0)

speaker.Speak("Robot Arm Connected.")

time.sleep(2)
dx = 0
while arm.connected:
    data = getMod(mod,dx)
    while data[5] == 0:
        sender(data)
        dx += 1
        arm.clean_error()
        time.sleep(.5)
#finish up:   
endAll()
