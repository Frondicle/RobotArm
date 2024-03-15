#generated with Grasshopper for Rhino
#Chris Morrey

import os
import sys
import time
import math

sys.path.append("C:\\Users\\morr0289\\Documents\\Repositories\\Github\\RobotArm\\Python\\final\\armFinal")

from .arm_sdk4.xarm.wrapper.xarm_api import XArmAPI
arm = XArmAPI('192.168.1.240')
hostId = 9
baudRate = 19200 #:param baud: 4800/9600/19200/38400/57600/115200/230400/460800/921600/1000000/1500000/2000000/2500000

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
line = 103

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
def sender(data_frame):
    global line
    global id
    arm.clean_error()
    arm.set_state(state=0)
    ret = arm.getset_tgpio_modbus_data(data_frame,host_id=hostId)
    if ret[0] == 0:
        print('wrote',data_frame[7],data_frame[8],data_frame[9],data_frame[10],data_frame[11],data_frame[12],sep='-')
        print('got back this: ',ret[1])
        print('line: ',line)
        line += 3
        time.sleep(.75)
    elif ret[0]!= 0:
        errors(ret[0])

arm.motion_enable(enable=True)
arm.set_mode(0)
arm.set_state(state=0)

arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) 
arm.set_tcp_offset([0,0,110,0,30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=110)

'''TGPIO for air solenoid---------------------------------------------
value = 0

    code = arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''


arm.reset(wait=True)
if arm.warn_code != 0:
    arm.clean_warn()
if arm.error_code != 0:
    arm.clean_error()

while arm.connected and arm.error_code ==0:
#data is big-endian
    #sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0xa4,0x01,0x1f,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,164,287,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x03,0x14,0x05,0x63,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,788,1379,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x06,0x78,0x0b,0x52,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,1656,2898,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0xa4,0x02,0xdf,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,420,735,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x07,0x14,0x0f,0xe3,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,2324,4067,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x66,0x02,0x72,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,358,626,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x0b,0x0b,0x13,0x53,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,2827,4947,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x48,0x02,0x3f,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,328,575,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x0c,0xa9,0x16,0x29,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,3241,5673,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x37,0x02,0x20,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,311,544,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x0e,0x0f,0x18,0x9a,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,3599,6298,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x2b,0x02,0x0c,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,299,524,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x0f,0x4a,0x1a,0xc2,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,3914,6850,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x23,0x01,0xfd,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,291,509,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x10,0x64,0x1c,0xb0,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,4196,7344,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x1d,0x01,0xf3,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,285,499,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x11,0x64,0x1e,0x6f,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,4452,7791,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x18,0x01,0xea,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,280,490,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x12,0x4d,0x20,0x07,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,4685,8199,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x14,0x01,0xe3,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,276,483,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x13,0x23,0x21,0x7d,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,4899,8573,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x11,0x01,0xde,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,273,478,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x13,0xe8,0x22,0xd6,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,5096,8918,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x0e,0x01,0xd9,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,270,473,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x14,0x9d,0x24,0x14,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,5277,9236,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x0c,0x01,0xd5,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,268,469,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x15,0x45,0x25,0x3a,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,5445,9530,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x0a,0x01,0xd2,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,266,466,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x15,0xe1,0x26,0x4b,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,5601,9803,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)
    datas = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x01,0x07,0x01,0xcf,0x00,0x01,0x00,0x01,0x00,0x01]
    #datas = [9,func16,start1,start2,numreg1,numreg2,numbytes,265,463,0x00,0x01,0x00,0x01,0x00,0x01]
    sender(datas)


arm.disconnect()
