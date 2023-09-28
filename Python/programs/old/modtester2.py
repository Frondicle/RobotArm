#code generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk2.xarm.wrapper.xarm_api import XArmAPI

arm = XArmAPI('192.168.1.196')
def u16_to_bytes(data):
    bts = bytes([data // 256 % 256])
    bts += bytes([data % 256])
    return bts

def sender(port,lengthbig,lengthsmall,speedbig,speedsmall,dir):
    hPort = u16_to_bytes(port)
    hLengthbig = u16_to_bytes(lengthbig)
    hLengthsmall = u16_to_bytes(lengthsmall)
    hSpeedbig = u16_to_bytes(speedbig)
    hSpeedsmall = u16_to_bytes(speedsmall)
    hdir=u16_to_bytes(dir)
    #code, ret = arm.getset_tgpio_modbus_data([port, 0x10, 0x00, 0x00, 0x00,0x05, 0x10,lengthbig,lengthsmall,speedbig,speedsmall,hdir],is_transparent_transmission=True)
    #code, ret = arm.getset_tgpio_modbus_data([port, 10, 0, 0, 0,5,16,lengthbig,lengthsmall,speedbig,speedsmall,hdir],is_transparent_transmission=True)
    code, ret = arm.getset_tgpio_modbus_data([lengthbig,lengthsmall,speedbig,speedsmall,dir],is_transparent_transmission=True,use_503_port=True)
    time.sleep(1)

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

arm.reset(wait=True)

time.sleep(2)

sender(9,226, 3,204,6,1)
sender(9,227, 3,206,6,0)
sender(9,229, 3,208,6,1)
sender(9,229, 3,209,6,0)
sender(9,226, 3,205,6,1)
sender(9,214, 3,182,6,0)
arm.reset(wait=True)
arm.disconnect()
