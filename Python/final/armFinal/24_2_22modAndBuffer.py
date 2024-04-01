#Chris Morrey 1/25/2023
#modbus and buffer only
import sys
import time
#from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import win32com.client
import logging 
logging.basicConfig(filename='output.txt',level=logging.DEBUG)

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\arm_sdk4")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShort.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='dex',usecols=['index','move','espeed','dir','end'])

#move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])

init = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00] #clear all registers, register 0 index to 1

end = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]#end of file flag to register 4

xArmDefaultSpeed = 50

goHome = [0,305,0,100,0,-180,0,xArmDefaultSpeed]

ip = '192.168.1.240'
arm = XArmAPI(ip)

speaker = win32com.client.Dispatch("SAPI.SpVoice")


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

def getExtruderFlag(char):
    if char =='bufferAvailable':
        x = arm.get_tgpio_digital(0) #extruder buffer available 
    elif char =='end':
        x = arm.get_tgpio_digital(1)
    return x

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

def getIndex():
    global arm
    data = arm.getset_tgpio_modbus_data([0x07,0x03,0x00,0x00,0x00,0x01],host_id=9)
    modResponse = data[1]
    print ('getIndex: modResponse: ',modResponse)
    dex = modResponse[3]
    pex = modResponse[4]
    lex = [dex,pex]
    rex = int.from_bytes(lex, 'big', signed=False)
    print ('from getIndex: ',rex,', code = ',data[0])
    return rex

def getBuffer():
    global arm
    data = arm.getset_tgpio_modbus_data([0x07,0x03,0x00,0x00,0x00,0x0a],host_id=9)
    print ('code: ',data [0],'data: ',data[1])
    modResponse = data[1]
    indexbig = modResponse[3]
    indexsmall = modResponse[4]
    movebig = modResponse[5]
    movesmall = modResponse[6]
    espeedbig = modResponse[7]
    espeedsmall = modResponse[8]
    dirbig = modResponse[9]
    dirsmall = modResponse[10]
    endbig = modResponse[11]
    endsmall = modResponse[12]
    index = int.from_bytes([indexbig,indexsmall], 'big', signed=False)
    move = int.from_bytes([movebig,movesmall], 'big', signed=False)
    espeed = int.from_bytes([espeedbig,espeedsmall], 'big', signed=False)
    dir = int.from_bytes([dirbig,dirsmall], 'big', signed=False)
    end = int.from_bytes([endbig,endsmall], 'big', signed=False)
    rex = [index,move,espeed,dir,end]
    ret = (data[0],rex)
    print ('from getBuffer: index:',rex[0],', move: ',rex[1],' speed: ',rex[2],' dir: ',rex[3],' end: ',rex[4],' code = ',data[0])
    return ret

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

'''def setArmReady(x):
    if x =='TRUE':
        arm.set_tgpio_digital(1,1)
        print ('setArmReady: arm ready')
    elif x =='FALSE':
        arm.set_tgpio_digital(1,0)
        print ('setArmReady: arm not ready')'''

def endAll(datas): #changed from original!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    sender(datas)
    time.sleep(2)
    speaker.speak("Shutting down extruder.")
    resetExtruder()
    #pd.ExcelFile.close(xlpath)
    arm.reset(wait=True)
    arm.set_state(state=4)
    arm.disconnect()

def packBuffer(last):   
    while getExtruderFlag('bufferAvailable') == 1:#finish this function!!!!!
        sendIndex = (last + 1)
        data=getMod(mod,sendIndex)
        sender(data)
        last = sendIndex
    return ret

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
#initial packet
sender(init)
print('init packet sent')
speaker.Speak("Initializing.")
time.sleep(3)
dx=1
while arm.connected:
        next = packBuffer(dx)
        stuff = getBuffer()
        if stuff[1][4]!= 1:
            dx = stuff[1][0]+1
            print('code:',stuff[0])
            print('stuff:',stuff[1])
            time.sleep(.25)
            arm.clean_error()
        else:
            print('end of file')
            break


#finish up:   
endAll(end)
