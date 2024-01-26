#Chris Morrey 11/28/2023

import sys
import time
#from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
#import win32com.client 


sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\arm_sdk4")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk4.xarm.wrapper import xarm_api #import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShort.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','move','espeed','dir','end'])

move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])

init = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00] #clear all registers

end = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]#end of file flag to register 4

xArmDefaultSpeed = 100

goHome = [305,0,100,0,-180,0,xArmDefaultSpeed]

xarm_api.arm = xarm_api.XArmAPI('192.168.1.240',is_radian=False)
id = 9
baudRate = 9600
if xarm_api.arm.warn_code != 0:
    xarm_api.arm.clean_warn()
if xarm_api.arm.error_code != 0:
    xarm_api.arm.clean_error()
ret = xarm_api.arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = xarm_api.arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)
xarm_api.arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
xarm_api.arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
xarm_api.arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
code, digitals = xarm_api.arm.get_tgpio_digital()

##speaker = win32com.client.Dispatch("SAPI.SpVoice")

#speaker.Speak("Robot Arm Connected.")

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
            xarm_api.arm.clean_error()
    else:
        print(module,'error not cleared; quitting...')
        time.sleep(2)
        quit()

def hexer(x):
    h = str(hex(x))
    s = h.lstrip("0x")
    e = s.zfill(4)
    f = e[0]
    g = e[1]
    h = e[2]
    i = e[3]
    j = "0x"
    k = ","
    a = j+f+g
    b = j+h+i
    c = [a,b]
    return c

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

def getMod(frame,ln):
    modData=frame.iloc[ln]
    indx = modData['index']
    move=modData['move']
    spd=modData['espeed']
    dir=modData['dir']
    end=modData['end']
    dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx, move, spd, dir, end]
    '''dframe = (0x07,0x10,0x00,0x00,0x00,0x04,0x08, move // 256 % 256, move // 256,spd // 256 % 256, spd // 256, dir // 256 % 256, dir // 256, end // 256 % 256,end // 256,)'''
    response = (dframe,indx)
    return response

def getIndex():
    global arm
    data = xarm_api.arm.getset_tgpio_modbus_data([0x07,0x03,0x00,0x00,0x00,0x01],host_id=7)
    modResponse = data[1]
    dex = modResponse[3]#get line from extruder
    print ('from getIndex: ',dex)
    return dex

'''def getQueueState():
    data_frame = [0x07,0x03,0x00,0x04,0x00,0x01]
    mod = xarm_api.arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    code,response = mod
    print('queue state: ',code,", ", response)
    return response'''

def sender(data_frame):
    global id
    code, digitals = xarm_api.arm.get_tgpio_digital()
    ret = xarm_api.arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    if ret[0] == 0:
        print('sender: data sent')
    else:
        print('sender: ',ret[0])

def mover(move):
    xarm_api.arm.get_err_warn_code(show=True, lang='en')
    xarm_api.arm.clean_warn()
    xarm_api.arm.clean_error()
    xarm_api.arm.set_state(state=0)
    xarm_api.arm.set_mode(mode=0)
    print(move)
    xarm_api.arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=move[7], is_radian=False, wait=True)

def getExtruderFlag(char):
    global arm
    if char =='ready':
        x = xarm_api.arm.get_tgpio_digital(0)
    elif char =='end':
        x = xarm_api.arm.get_tgpio_digital(1)
    return x

def resetExtruder():
    xarm_api.arm.set_tgpio_digital(0,0)
    time.sleep(.2)
    xarm_api.arm.set_tgpio_digital(0,1)

def setArmReady(x):
    if x =='TRUE':xarm_api.arm.set_tgpio_digital(1,1)
    elif x =='FALSE':xarm_api.arm.set_tgpio_digital(1,0)

def endAll(datas,lastMove):
    sender(datas)
    mover(lastMove)
    time.sleep(2)
    resetExtruder()
    pd.ExcelFile.close(xlpath)
    xarm_api.arm.reset(wait=True)
    xarm_api.arm.set_state(state=4)
    xarm_api.arm.disconnect()

'''TGPIO for air solenoid---------------------------------------------
value = 0
    code = xarm_api.arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = xarm_api.arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''

xarm_api.arm.reset(wait=True)
xarm_api.arm.motion_enable(enable=True)
xarm_api.arm.get_err_warn_code(show=True, lang='en')
xarm_api.arm.clean_warn()
xarm_api.arm.clean_error()
time.sleep(2)

#initial packet:
sender(init)
print('init packet sent')
#speaker.Speak("Initializing.")
dataRow = 0
while xarm_api.arm.connected:
    while dataRow <= mod.size / 5:
        print ('row: ',dataRow)
        xarm_api.arm.clean_error()
        xarm_api.arm.motion_enable(True)
        setArmReady('TRUE')
        if (getExtruderFlag('ready') == 1): 
            mv = getIndex()
            datas = getMod(mod,mv)   
            newMove = getMove(move,mv)
            sender(datas[0])
            mover(move)
            dataRow += 1
        elif (getExtruderFlag('ready') == 0):
            time.sleep(.05)
#finish up:   
endAll(end,goHome)
