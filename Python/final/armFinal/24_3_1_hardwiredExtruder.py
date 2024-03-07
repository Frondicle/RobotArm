'''Chris Morrey 3/1/2024
RobotArm project: hardwired extruder configuration:
Python control of Ufactory xArm 6 with extruder hardwired external to the arm mechanism.'''

import sys
import time
import pySerial
import numpy as np
import pandas as pd
import win32com.client
import logging 
logging.basicConfig(filename='output.txt',level=logging.DEBUG)

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\arm_sdk4")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShort.xlsx"

mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','move','espeed','dir','end'])

move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])


xArmDefaultSpeed = 50

goHome = [0,305,0,100,0,-180,0,xArmDefaultSpeed]

ip = '192.168.1.240'
arm = XArmAPI(ip)

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

def mover(move):
    arm.get_err_warn_code(show=True, lang='en')
    arm.clean_warn()
    arm.clean_error()
    arm.set_state(state=0)
    print('move: ',move)
    arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=move[7], is_radian=False, wait=True)
    arm.clean_error()

def getExtruderFlag(char):
    global arm
    if char =='ready':
        x = arm.get_tgpio_digital(0)
    elif char =='end':
        x = arm.get_tgpio_digital(1)
    return x

def resetExtruder():
    arm.set_tgpio_digital(0,1)
    time.sleep(1)
    arm.set_tgpio_digital(0,0)

def endAll(lastMove):
    mover(lastMove)
    time.sleep(2)
    resetExtruder()
    #pd.ExcelFile.close(xlpath)
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
'''
arm.reset(wait=True)
arm.motion_enable(enable=True)
arm.get_err_warn_code(show=True, lang='en')
arm.clean_warn()
arm.clean_error()
time.sleep(2)'''

speaker = win32com.client.Dispatch("SAPI.SpVoice")

speaker.speak("Resetting Extruder.")
resetExtruder()
time.sleep(3)

arm.motion_enable(True)
arm.clean_error()
arm.set_mode(0)
arm.set_state(state = 0)
arm.reset(wait=True)
time.sleep(1)

time.sleep(1)
#arm.set_state(state=0)
arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
code, digitals = arm.get_tgpio_digital()

speaker.Speak("Robot Arm Connected.")

time.sleep(5)
speaker.Speak("Initializing.")
time.sleep(1)
while arm.connected:
        while mv <= mod.size / 5:
            print ('row: ',mv)
            arm.clean_error()
            #arm.motion_enable(True)
            if (getExtruderFlag('ready') == 1):
                print('extruder ready') 
                print('mv: ',mv) 
                newMove = getMove(move,mv)
                mover(move)
            elif (getExtruderFlag('ready') == 0):
                time.sleep(.05)
#finish up:   
endAll(goHome)
