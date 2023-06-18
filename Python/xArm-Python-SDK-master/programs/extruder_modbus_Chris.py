#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2020, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>

"""
Example: Control the bio gripper through the modbus of the end tool
"""

import os
import sys
import time
sys.path.append(os.path.join(os.path.dirname(__file__), '../../..'))

from xarm.wrapper import XArmAPI
from configparser import ConfigParser
parser = ConfigParser()
parser.read('../robot.conf')
try:
    ip = parser.get('xArm', 'ip')
except:
    ip = input('Please input the xArm ip address[192.168.1.196]:')
    if not ip:
        ip = '192.168.1.196'


arm = XArmAPI(ip)
time.sleep(0.5)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

arm.motion_enable(True)
arm.set_mode(0)
arm.set_state(0)

code = arm.set_tgpio_modbus_timeout(20)
print('set_tgpio_modbus_timeout, code={}'.format(code))

# set tool gpio modbus baudrate
code = arm.set_tgpio_modbus_baudrate(115200)
print('set_tgpio_modbus_baudrate, code={}'.format(code))
time.sleep(1)


while arm.connected and arm.error_code == 0:
    ret = arm.core.tgpio_set_modbus(data_frame, len(data_frame))([0x08, 0x10, 0x00, 0x01, 0x00, 0x03, 0x06, 0x00, (move), 0x00, (dir), 0x00, (speed)]) #setup these variables as hex values

    time.sleep(2)

