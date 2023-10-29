#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2019, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>

"""
Example: Set Tool GPIO Digital
"""

import os
import sys
import time

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI
from configparser import ConfigParser
parser = ConfigParser()
parser.read('../robot.conf')
ip = '192.168.1.196' 
IO = 1

arm = XArmAPI(ip, check_robot_sn=False)
time.sleep(0.5)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

for i in range (10):
    print('set IO high level')
    arm.set_tgpio_digital(IO, 1)
    time.sleep(2)
    print('set IO high level')
    arm.set_tgpio_digital(IO, 0)
    time.sleep(2)



