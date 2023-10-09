#generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math
#import sender, buildFrame, errors

data = open('C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\programs\\ext_data.txt','r', buffering=1)

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI
arm = XArmAPI('192.168.1.196')
id = 9
baudRate = 9600

def sender(data_frame):
    global line
    global id
    arm.clean_error()
    arm.set_state(state=0)
    code, digitals = arm.get_tgpio_digital()
    #while arm.connected and arm.error_code != 19 and arm.error_code != 28:
    if code == 0 and digitals[0] == 1:
        print ('extruder reports ready')
        ret = arm.getset_tgpio_modbus_data(data_frame,host_id=id)
        if ret[0] == 0:
            print('wrote ',data_frame[8])
            print('line: ',line)
            line += 3
            time.sleep(.75)
    elif digitals [0] == 0:
        print('extruder reports not ready')
    elif code != 0:
        errors(ret[0])

def buildFrame(filedata,ln):
    moves = filedata.read(ln)
    print(moves)
    frame = ('0x09,0x10,0x00,0x00,0x00,0x03,0x06,',moves)
    return frame

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
line = 1

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) 
arm.set_tcp_offset([0,0,110,0,30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=110)

last_digitals = [-1, -1]

arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

while arm.connected and arm.error_code ==0:
    for x in data:
        frame = buildFrame(data,line)
        sender(frame)
        time.sleep(.25)
        line += 1

arm.disconnect()
