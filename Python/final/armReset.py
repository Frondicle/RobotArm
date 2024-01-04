import sys
import time
from urllib import response
import win32com.client 

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from arm_sdk3.xarm.wrapper.xarm_api import XArmAPI

arm = XArmAPI('192.168.1.240',is_radian=False)
id = 9
baudRate = 9600

speaker = win32com.client.Dispatch("SAPI.SpVoice")
code, digitals = arm.get_tgpio_digital()

if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()
time.sleep(2)

ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))

time.sleep(1)

def errors(module,returned):
    if returned == 1:
        print(module,'1: uncleared error exists')
    elif returned == 23:
        print(module,'23:modbus reply length error')
    elif returned == 20:
        print(module,'20: wrong slave id')
    elif returned == 15:
        print(module,'15: Modbus commands full')
    elif returned == 19:
        print(module,'19: communication error')
    elif returned == 28:
        print(module,'28: end module communication error')
    elif returned != 0:
        print(module,'other error: ',returned)
    input=('clear errors? (y/n)')
    time.sleep(5)
    if input == 'y':
            arm.clean_error()
    else:
        print(module,'error not cleared; quitting...')
        time.sleep(2)
        quit()
        
def extruderReset():
    arm.set_tgpio_digital(0, 1, delay_sec=None)
    time.sleep(1)
    arm.set_tgpio_digital(0, 0, delay_sec=None)
    print('extruder reset')
    time.sleep(1)

extruderReset
time.sleep(2)
extruderReset
arm.reset(wait=True)
arm.disconnect()
