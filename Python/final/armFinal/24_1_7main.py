#Chris Morrey 11/28/2023

import sys
import time
#from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import win32com.client 


sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShort.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','line','move','espeed','dir','end'])

move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','line','x','y','z','pitch','roll','yaw','fspeed'])

init = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00] #clear all registers

end = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]#end of file flag to register 4

xArmDefaultSpeed = 100

goHome = [305,0,100,0,-180,0,xArmDefaultSpeed]

arm = XArmAPI('192.168.1.240',is_radian=False)
id = 9
baudRate = 9600
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)
arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
code, digitals = arm.get_tgpio_digital()

speaker = win32com.client.Dispatch("SAPI.SpVoice")

speaker.Speak("Robot Arm Connected.")

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

def naive_conversion(x, base):
    t = ""
    if x==0:
        ab = 0
        cd = 0
        return cd,ab
    else:
        while x > 0:
            t = str(x % base) + t
            x //= base
        t=t.zfill(4) 
        ab = (t[2],t[3])
        abj = int("".join(ab))
        cd = (t[0],t[1])
    cdj = int("".join(cd))
    ret=cdj,abj
    return ret

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
    pti = move['line']
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
    indx = naive_conversion(modData['line'],16)
    print ('index: ',indx)
    move=naive_conversion(modData['move'],16)
    spd=naive_conversion(modData['espeed'],16)
    dir=naive_conversion(modData['dir'],16)
    end=naive_conversion(modData['end'],16)
    dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx[0], indx[1],move[0],move[1], spd[0],spd[1], dir[0],dir[1], end[0],end[1]]
    '''dframe = (0x07,0x10,0x00,0x00,0x00,0x04,0x08, move // 256 % 256, move // 256,spd // 256 % 256, spd // 256, dir // 256 % 256, dir // 256, end // 256 % 256,end // 256,)'''
    response = (dframe,indx)
    return response

def getIndex():
    global arm
    data = arm.getset_tgpio_modbus_data([0x07,0x03,0x00,0x00,0x00,0x01],host_id=9)
    print ('from getIndex data: ',data)
    modResponse = data[1]
    dex = ((modResponse[3]*256)+modResponse[4])#get line from extruder
    print ('from getIndex: ',dex)
    return dex

def sender(data_frame):
    print(data_frame)
    global id
    ret = arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    if ret[0] == 0:
        print('sender: data sent')
    else:
        print('sender: ',ret[0])

def mover(move):
    arm.get_err_warn_code(show=True, lang='en')
    arm.clean_warn()
    arm.clean_error()
    arm.set_state(state=0)
    arm.set_mode(mode=0)
    print(move)
    s=(move[7]*.1)
    arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=s, is_radian=False, wait=True)

def getExtruderFlag(char):
    global arm
    if char =='ready':
        x = arm.get_tgpio_digital(0)
    elif char =='end':
        x = arm.get_tgpio_digital(1)
    print ('extruder flag errcodes: ',x[0])
    print ('extruder flag: ',x[1])
    return x[1]

def resetExtruder():
    arm.set_tgpio_digital(0,0)
    time.sleep(.2)
    arm.set_tgpio_digital(0,1)

def setArmReady(x):
    if x =='TRUE':arm.set_tgpio_digital(1,1)
    elif x =='FALSE':arm.set_tgpio_digital(1,0)

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

arm.reset(wait=True)
arm.motion_enable(enable=True)
arm.get_err_warn_code(show=True, lang='en')
arm.clean_warn()
arm.clean_error()
time.sleep(2)

#initial packet:
sender(init)
print('init packet sent')
speaker.Speak("Initializing.")
r = getExtruderFlag('ready')
e = getExtruderFlag('end')
print ('modbus queue: ',mod.size)
time.sleep(2)
dataRow = 0
while arm.connected:
    arm.motion_enable(True)
    while dataRow <= mod.size / 5:
        print ('row: ',dataRow)
        arm.clean_error()
        setArmReady('TRUE')
        if (getExtruderFlag('ready') == 1): 
            print('extruder ready')
            mv = getIndex()
            datas = getMod(mod,mv)   
            newMove = getMove(move,mv)
            sender(datas[0])
            mover(newMove)
            dataRow += 1
        elif (getExtruderFlag('ready') == 0):
            print('extruder not ready')
            time.sleep(.05)
#finish up:   
endAll(end,goHome)
