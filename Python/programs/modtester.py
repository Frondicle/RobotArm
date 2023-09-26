#code generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math


sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk2.xarm.wrapper.xarm_api import XArmAPI



def sender(port,length,speed):
    code, ret = arm.getset_tgpio_modbus_data(['port', 0x10, 0x00, 0xc8, 0x00, 0x04, 0x08, 0x00, 'length','speed'],is_transparent_transmission=True)

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

arm.reset(wait=True)