import os
import sys
import time
sys.path.append(os.path.join(os.path.dirname(__file__), '../../..'))

# Receive variables from RoboDK
#move = RDK.getParam('move')  # Make sure the 'move' parameter is set correctly in RoboDK
#dir = RDK.getParam('dir')    # Make sure the 'dir' parameter is set correctly in RoboDK
#speed = RDK.getParam('speed')  # Make sure the 'speed' parameter is set correctly in RoboDK

from xarm.wrapper import xarm_api
from configparser import ConfigParser

parser = ConfigParser()
parser.read('../robot.conf')

try:
    ip = parser.get('xArm', 'ip')
except:
    print("Failed to read IP from robot.conf")
    sys.exit(0)

arm = xarm_api(ip)
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

# Set tool gpio modbus baudrate
code = arm.set_tgpio_modbus_baudrate(115200)
print('set_tgpio_modbus_baudrate, code={}'.format(code))

time.sleep(1)

data_frame = [0x08, 0x10, 0x00, 0x01, 0x00, 0x03, 0x06, 0x00, move, 0x00, dir, 0x00, speed]
ret = arm.core.tgpio_set_modbus(data_frame, len(data_frame))  # Setup these variables as hex values

time.sleep(2)

