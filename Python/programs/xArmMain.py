#generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI
arm = XArmAPI('192.168.1.196')
id = 9
baudRate = 9600
arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) 
arm.set_tcp_offset([0,0,110,0,30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=110)

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
line = 0

def sender(data_frame):
    global line
    arm.clean_error()
    arm.set_state(state=0)
    code, digitals = arm.get_tgpio_digital()
    print
    while arm.connected and arm.error_code != 19 and arm.error_code != 28:
        if code == 0 and digitals[0] == 1:
            print ('extruder reports ready')
            ret = arm.getset_tgpio_modbus_data(data_frame,host_id=id)
            if digitals [0] == 0:
                print('extruder reports not ready')
            if ret[0] == 1:
                print('uncleared error exists')
            if ret[0] == 0:
                print('wrote some stuff')
            if ret[0] == 23:
                print('wrong reply length: ',len(ret[1]),' bytes')
            if ret[0] == 20:
                print('wrong slave id')
            if ret[0] == 15:
                print('Modbus commands full')
            if ret[0] == 19:
                print('communication error')
            if ret[0] == 28:
                print('communication error')
            print(len(ret[1]))
            line += 1
            time.sleep(.75)

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

last_digitals = [-1, -1]


arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

while arm.connected and arm.error_code ==0:

    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x00, 0xa4,0x01, 0x1f,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x03, 0x14,0x05, 0x63,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x06, 0x78,0x0b, 0x52,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x01, 0xa4,0x02, 0xdf,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x09, 0x14,0x0f, 0xe3,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x01, 0x66,0x02, 0x72,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x0b, 0x0b,0x13, 0x53,0x00,0x01]
    sender(datas)
    datas = [0x09,0x10,0x00,0x00,0x00,0x03,0x06,0x01, 0x48,0x02, 0x3f,0x00,0x01]
    sender(datas)

arm.disconnect()
