#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2019, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>

"""
Description: Move Circle
"""

import os
import sys
import time

sys.path.append(os.path.join(os.path.dirname(__file__), '../../..'))

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI


#######################################################
ip = "192.168.1.240"

########################################################


arm = XArmAPI(ip)


arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) 
arm.set_tcp_offset([0,0,80,0,0,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=80)



poses = [
    [300,  0,   -.5, -180, 0, 0],
    [300,  100, -.5, -180, 0, 0],
    [400,  100, -.5, -180, 0, 0],
    [400, -100, -.5, -180, 0, 0],
    [300,  0,   -.5, -180, 0, 0]
]

arm.clean_error()
arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

ret = arm.set_position(*poses[0], speed=50, mvacc=100, wait=False)
print('set_position, ret: {}'.format(ret))

arm.clean_error()
arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)
ret = arm.move_circle(pose1=poses[1], pose2=poses[2], percent=100, speed=100, mvacc=1000, wait=True)
print('move_circle, ret: {}'.format(ret))

arm.clean_error()
arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)
ret = arm.move_circle(pose1=poses[3], pose2=poses[4], percent=100, speed=100, mvacc=1000, wait=True)
print('move_circle, ret: {}'.format(ret))

arm.reset(wait=True)
arm.disconnect()
