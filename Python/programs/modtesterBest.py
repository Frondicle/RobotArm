#code generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk2.xarm.wrapper.xarm_api import XArmAPI


arm = XArmAPI('192.168.1.196')

time.sleep(2)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

arm.motion_enable(True)
arm.set_mode(0)
arm.set_state(0)
version = arm.get_tgpio_version()
print ('tgpio version: ',version)
arm.set_tgpio_modbus_timeout(3)
baudResponse = arm.get_tgpio_modbus_baudrate()
print('the bauds are: ',baudResponse)

code = arm.set_tgpio_modbus_baudrate(9600)
print('set_tgpio_modbus_baudrate, code={}'.format(code))

time.sleep(2)
arm.clean_error()
print('slept')

def sender(data_frame):
    code, ret = arm.getset_tgpio_modbus_data(data_frame,min_res_len=15,host_id=9,is_transparent_transmission=True, use_503_port=True)
    print('send data',len(data_frame),' bytes')
    if code == 1:
        print('uncleared error exists')
    if code == 0:
        print('seems good')
    if code == 23:
        print('wrong reply length: ',len(ret),' bytes')
    if code == 20:
        print('wrong slave id')
    if code == 15:
        print('Modbus commands full')
    if code == 19:
        print('communication error')
    if code == 28:
        print('communication error')
   
    #code, ret = arm.getset_tgpio_modbus_data(data_frame,min_res_len=0, host_id=9)
    time.sleep(1)

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

time.sleep(2)

while arm.connected:
    datas =(0x09,0x10,0x00,0x00,0x00,0x05,0x0a,0xd6,0x03,0xb6,0x00)
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
