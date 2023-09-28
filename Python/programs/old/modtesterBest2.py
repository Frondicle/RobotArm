#code generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI


arm = XArmAPI('192.168.1.196')

#arm.set_tcp_load()  DO THIS*****************************
#arm.set_tcp_offset()
#arm.set_collision_tool_model
arm.motion_enable(True)
arm.set_mode(0)
arm.set_state(0)

if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

ret = arm.core.set_modbus_timeout(5)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(9600)  
print('set modbus baudrate, ret = %d' % (ret[0]))    

   

def sender(data_frame):
    ret = arm.core.getset_tgpio_modbus_data(data_frame,len(data_frame))
    print('set modbus, ret = %d' % (ret[0]))


arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

time.sleep(2)

while arm.connected:
    datas=[0x09, 0x10, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x03, 0x00, 0x02,0x00,0x01] 
    sender(datas)
    arm.clean_error()
    datas =(0x09,0x10,0x00,0x05,0x0a,0xd7,0x03,0xb7,0x01)
    sender(datas)
    arm.clean_error()
    datas =(0x09,0x10,0x00,0x05,0x0a,0xd8,0x03,0xb8,0x06,0x00)
    sender(datas)
    arm.clean_error()
    datas =(0x09,0x10,0x00,0x05,0x0a,0xd9,0x03,0xb9,0x06,0x01)
    sender(datas)
    arm.clean_error()
    datas =(0x09,0x10,0x00,0x05,0x0a,0xda,0x03,0xba,0x06,0x00)
    sender(datas)
    arm.clean_error()
    datas =(0x09,0x10,0x00,0x05,0x0a,0xdb,0x03,0xbb,0x06,0x01)
    sender(datas)
arm.reset(wait=True)
arm.disconnect()
