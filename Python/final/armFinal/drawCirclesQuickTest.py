#generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI
arm = XArmAPI('192.168.1.240')

code, digitals = arm.get_tgpio_digital()

if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

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

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) 
arm.set_tcp_offset([0,0,80,0,0,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=80)

arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)


while arm.connected:
    arm.set_state(state=0)
    arm.move_gohome(is_radian=False, wait=True)

    arm.set_position(x=250.00, y=250.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=0.00, y=300.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=-250.00, y=250.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=-300.00, y=0.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=-250.00, y=-250.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=0.00, y=-300.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=250.00, y=-250.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)    
    arm.set_position(x=300.00, y=0.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.set_position(x=300.00, y=0.00, z=-1, roll=-180.00, pitch=0.00, yaw=0.00, speed=50.00, is_radian=False, wait=True)
    arm.move_gohome
    time.sleep(2)
    arm.reset(wait=True)
    arm.disconnect()
#finish up:   

