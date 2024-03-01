#generated with Grasshopper for Rhino
#Chris Morrey


import sys
import time

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI
arm = XArmAPI('192.168.1.240')

code, digitals = arm.get_tgpio_digital()
arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

while arm.connected:
    arm.set_state(state=0)
    arm.move_gohome

time.sleep(2)
arm.reset(wait=True)
arm.disconnect()