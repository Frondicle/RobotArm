'''Chris Morrey 1/2/2024
modules for robot arm control

Modbus format:
0x07=slave id
0x10=function 16 write registers, function 3 read registers
0x00,0x00=beginning address to write
0x00,0x05 = number of registers
0x0a=number of bytes(2x registers)
0x00,0x15=register 0 index
0x00,0x06=register 1 steps
0x00,0x00=register 2 espeed
0x00,0x01=register 3 direction 1=forward 0=reverse
0x00,0x00=register 4 end of file 1=stop 0=continue
0x00,0x00=register 5 spare
Digitals:
TI0: ready flag: 1=ready, 0=not ready
TI1: start flag:
TO0: reset extruder: 0=reset, 1=normal
TO1: 
'''

#sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")
#import sys
import time
#from urllib import response
#from unicodedata import digit
#import numpy as np
import pandas as pd
import win32com.client 
from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI
from define import *

#Initialize arm stuff--------------------------------------------
def armStart():
    global arm
    arm = XArmAPI(xArmIP,is_radian=False)
    id = xArmSlaveID
    baudRate = xArmBaudRate
    arm.set_mode(0)
    ret = arm.core.set_modbus_timeout(7)  
    print('set modbus timeout, ret = %d' % (ret[0]))    
    ret = arm.core.set_modbus_baudrate(baudRate)  
    print('set modbus baudrate, ret = %d' % (ret[0]))
    time.sleep(1)
    arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
    arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
    arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
    if arm.warn_code != 0:
        arm.clean_warn()
    if arm.error_code != 0:
        arm.clean_error()
    ret=[arm.warn_code,arm.error_code]
    time.sleep(1)
    return ret

def initSpeech():#text to speech----------------------------------- 
    if (speak == 'TRUE'):
        speaker = win32com.client.Dispatch("SAPI.SpVoice")
    elif (speak == 'FALSE'):
        speaker = 0
    return speaker

#Excel file reader------------------------------------------------
def makeFrames(path):
    ext = pd.read_excel(path, "1", index_col='index',usecols=['index','steps','espeed','dir','end'])
    arm = pd.read_excel(path, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])
    return ext,arm

#Error interpreter-------------------------------------------------
def errors(module,returned):
    if returned == 1:
        print(module,'1: uncleared error exists')
    elif returned == 23:
        print(module,'23:modbus reply length error')
    elif returned == 20:
        print(module,'20: wrong slave id')
    elif returned == 15:
        print(module,'15: Modbus commands full')
    elif returned == 19:
        print(module,'19: communication error')
    elif returned == 28:
        print(module,'28: end module communication error')
    elif returned != 0:
        print(module,'other error: ',returned)
    input=('clear errors? (y/n)')
    time.sleep(5)
    if input == 'y':
            arm.clean_error()
    else:
        print(module,'error not cleared; quitting...')
        time.sleep(2)
        quit()

#xArm movement data frame interpreter----------------------------------
def getMove(frame,ln):
    move = frame.iloc[ln]
    pti = move['index']
    ptx = move['x']
    pty = move['y']
    ptz = move['z']
    ptp = move['pitch']
    ptr = move['roll']
    ptyaw = move['yaw']
    pts = move['fspeed']
    mv = [pti,ptx,pty,ptz,ptp,ptr,ptyaw,pts]
    return mv

#Extruder data frame interpreter--------------------------------------
def getMod(frame,ln):
    modData=frame.iloc[ln]
    indx = modData['index']
    stps=modData['steps']
    spd=modData['espeed']
    dir=modData['dir']
    end=modData['end']
    dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx, stps, spd, dir, end]
    response = (dframe,indx)
    return response

#Modbus commands to extruder----------------------------------------
def sender(data_frame):
    global id
    code, digitals = arm.get_tgpio_digital()
    #queueState = getQueueState
    #print ('queue state: ',queueState)
    #while queueState == 1 and code != 19 and code != 28:
    #while arm.connected and arm.error_code != 19 and arm.error_code != 28:
    ret = arm.getset_tgpio_modbus_data(data_frame,host_id=xArmSlaveID)
    if ret[0] == 0:
        print('sender: data sent')
    else:
        print('sender: ',ret[0])

#Movement information to xArm---------------------------------------
def mover(move):
    arm.get_err_warn_code(show=True, lang='en')
    arm.clean_warn()
    arm.clean_error()
    arm.set_state(state=0)
    arm.set_mode(mode=0)
    print(move)
    arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=move[7], is_radian=False, wait=True)
    while arm.ismoving():
        setArmReady('FALSE')

def getIndex():
    data = arm.getset_tgpio_modbus_data([0x07,0x03,0x00,0x00,0x00,0x01],host_id=7)
    modResponse = data[1]
    dex = modResponse[3]#get line from extruder
    print ('from getIndex: ',dex)
    return dex

#if __name__ == '__main__':
#    sys.exit(main())  # next section explains the use of sys.exit


#get digital flags from extruder-------------------------------------
#params: 'ready' or 'end'
def getExtruderFlag(char):
    if char =='ready':
        x = arm.get_tgpio_digital(0)
    elif char =='end':
        x = arm.get_tgpio_digital(1)
    return x

def resetExtruder():
    arm.set_tgpio_digital(0,0)
    time.sleep(.2)
    arm.set_tgpio_digital(0,1)

def setArmReady(x):
    if x =='TRUE':arm.set_tgpio_digital(1,1)
    elif x =='FALSE':arm.set_tgpio_digital(1,0)


#end of file actions-------------------------------------------------
#params: modbus end packet []
def endAll(datas,lastMove):
    sender(datas)
    mover(lastMove)
    time.sleep(2)
    resetExtruder()
    pd.ExcelFile.close(xlpath)
    arm.reset(wait=True)
    arm.set_state(state=4)
    arm.disconnect()

    '''TGPIO for air solenoid---------------------------------------------
value = 0
    code = arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''
