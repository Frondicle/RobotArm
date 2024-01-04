#Chris Morrey 11/28/2023

import sys
import time
#from unicodedata import digit
import numpy as np
import pandas as pd
import win32com.client 

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI

arm = XArmAPI('192.168.1.240',is_radian=False)
id = 9
baudRate = 9600

speaker = win32com.client.Dispatch("SAPI.SpVoice")

xlpath = "C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\grassDataShort.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','move','espeed','dir','end'])

move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])

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

def errors(returned):
    if returned == 1:
        print('1: uncleared error exists')
    elif returned == 23:
        print('23:modbus reply length error')
    elif returned == 20:
        print('20: wrong slave id')
    elif returned == 15:
        print('15: Modbus commands full')
    elif returned == 19:
        print('19: communication error')
    elif returned == 28:
        print('28: end module communication error')
    else:
        print('other error: ',returned)
    input=('clear errors? (y/n)')
    if input == 'y':
            arm.clean_error()
    else:
        print('error not cleared; quitting...')
        time.sleep(2)
        quit()

def setline(): 
    global line
    line = 
    return line

def getMove(frame,ln):
    move = frame.iloc[ln] 
    ptx = move['x']
    pty = move['y']
    ptz = move['z']
    ptp = move['pitch']
    ptr = move['roll']
    ptyaw = move['yaw']
    pts = move['fspeed']
    mv = [ptx,pty,ptz,ptp,ptr,ptyaw,pts]
    return mv

def modFrame(frame,ln):
    modData=frame.iloc[ln]
    move=modData['move']
    spd=modData['espeed']
    dir=modData['dir']
    end=modData['end']
    dframe = (0x09,0x10,0x00,0x00,0x00,0x04,0x08,move,spd,dir,end)
    return dframe

def sender(data_frame):
    code, digitals = arm.get_tgpio_digital()
    global id
    arm.clean_error()
    arm.motion_enable(enable=True)
    arm.set_state(state=0)
    time.sleep(.01)
    #while arm.connected and arm.error_code != 19 and arm.error_code != 28:
    if code == 0 and digitals[0] == 1:
        ret = arm.getset_tgpio_modbus_data(data_frame,host_id=id)
        if ret[0] == 0:
            print('data sent')  
    elif code != 0:
        errors(ret[0])

def mover(move):
    print(arm.get_err_warn_code())
    arm.clean_error()
    arm.motion_enable(enable=True)
    arm.set_state(state=0)
    time.sleep(.01)
    arm.set_position(x=move[0], y=move[1], z=move[2], roll=move[4], pitch=move[3], yaw=move[5], speed=move[6], is_radian=False, wait=True)

speaker.Speak("Robot Arm Connected.")


arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)

armspeed = 300

'''TGPIO for air solenoid---------------------------------------------
value = 0
    code = arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''

arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

'''datas format: [0x09: slave id,0x10 function 16 write registers,0x00,0x00 beginning address to write,0x00,0x05 number of registers,0x0a number of bytes(2x registers),0x00,0x15 register 0 line,0x00,0x06 register 1 movement,0x00,0x01 register 2 speed,0x00,0x64 register 3 direction,0x00,0x00 register 4 end of file, 0x00,0x03 line jump interval(only set at the beginning)]'''

line = 0

 #initial packet:
init = [0x09,0x10,0x00,0x00,0x00,0x06,0x0c,0x00,0x6f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02]
sender(init)
print('init packet sent')
speaker.Speak("Initializing.")

while arm.connected:
    for x in mod:
        line = setline()
        datas = modFrame(mod,line)
        newMove = getMove(move,line)
        arm.set_state(state=0)
        sender(datas)
        mover(newMove)
        #print(datas,newMove[0:6],sep=', ')
#finish up:   
datas =[0x09,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]
sender(datas,line)

time.sleep(2)
pd.ExcelFile.close(xlpath)
arm.reset(wait=True)
arm.disconnect()
