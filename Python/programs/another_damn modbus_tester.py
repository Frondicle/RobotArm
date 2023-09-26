import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\")

from arm_sdk2.xarm.wrapper.xarm_api import XArmAPI

arm = XArmAPI('192.168.1.196', is_radian=False, do_not_open=False)
time.sleep(0.5)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

arm.motion_enable(True)
arm.set_mode(0)
arm.set_state(0)

# code = arm.set_tgpio_modbus_timeout(20)
# print('set_tgpio_modbus_timeout, code={}'.format(code))

# set tool gpio modbus baudrate
code = arm.set_tgpio_modbus_baudrate(9600)
print('set_tgpio_modbus_baudrate, code={}'.format(code))

time.sleep(2)
data_set = [0x09, 0x10, 0x00, 0x05, 0xd6, 0x03, 0xb6, 0x06, 0x00]
while arm.connected and arm.error_code == 0:
    code, ret = arm.getset_tgpio_modbus_data(data_set)

time.sleep(2)
arm.reset(wait=True)
arm.disconnect()


