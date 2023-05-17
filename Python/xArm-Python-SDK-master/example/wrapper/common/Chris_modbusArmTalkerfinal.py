#!/usr/bin/env python3  
# Software License Agreement (BSD License)  
#
# Copyright (c) 2019, UFACTORY, Inc. 
# All rights reserved.  
#  
# Author: Vinman and then Chris did some    

"""  
Wasp extruder: 
200 steps = 1 rotation
Barrel diameter: 10mm 
area: 78.54mm2
Auger pitch: 7mm
	549.78 mm3 : 1 rotation
Nozzle diameter: 2mm
	Area: 3.14 mm2: 
	1 rotation = 175.09mm long extrusion
 
"""    
import os  
import sys  
import time  
sys.path.append(os.path.join(os.path.dirname(__file__), '../../..')) 
sys.path.append("C:\\Users\\morr0289\\Documents\\Robot_local\\RoboDK\\xArm_Python_SDK_master")
from xarm.wrapper import XArm_api
from configparser import ConfigParser  
parser = ConfigParser()  
parser.read('../robot.conf')  
try:      
   ip = parser.get('xArm', 'ip')  
except:      
    ip = input('Please input the xArm ip address[192.168.1.196]:')
if not ip:
    ip = '192.168.1.196'  

arm = XArm_api.XArmAPI(ip,is_radians=False,do_not_open=False)
time.sleep(0.5)  

if arm.warn_code != 0:      
    arm.clean_warn()  
if arm.error_code != 0:      
    arm.clean_error()    

ret = arm.core.set_modbus_timeout(5)
print('set modbus timeout, ret = %d' % (ret[0]))    
ret = arm.core.set_modbus_baudrate(9600)
print('set modbus baudrate, ret = %d' % (ret[0]))    
 
spd=150#sys.argv[1](0x9B)
dir=1#sys.argv[2]

# modbus to Arduino test
data_frame = [0x2A, 0x10, 0x00, 0x7C, 0x00, 0x02, 0x00, 0x04, spd // 256 % 256, spd % 256, dir // 256 % 256, dir % 256]
#data_frame = [0x2A, 0x10, 0x00, 0x06, 0x03, 0x9B, 0x01]
#data_frame = [0x10, 0x00, 0x00, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x9B, 0x00,0x01]
ret = arm.core.tgpio_set_modbus(data_frame, len(data_frame))  
print('set modbus, ret = %d' % (ret[0]))  
time.sleep(2)
print('modbus data finished')  

arm.disconnect() 
