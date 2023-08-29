#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# This script demonstrates how to control the bio gripper through the modbus of the end tool.

# Import required modules and libraries
import os  # Provides functions for interacting with the operating system
import sys  # Provides access to some variables used or maintained by the interpreter
import time  # Provides various time-related functions
sys.path.append(os.path.join(os.path.dirname(__file__), '../../..'))  # Append the path of the xArm library to the system path

from xarm.wrapper import XArmAPI  # Import the xArmAPI from the xarm.wrapper module
from configparser import ConfigParser  # Import the ConfigParser class from the configparser module

# Read robot configuration from the 'robot.conf' file
parser = ConfigParser()  # Create an instance of the ConfigParser class
parser.read('../robot.conf')  # Read the robot configuration from the 'robot.conf' file

try:
    ip = parser.get('xArm', 'ip')  # Get the IP address of the xArm from the configuration file
except:
    # Prompt for user input if IP address is not specified in the configuration file
    ip = input('Please input the xArm IP address [192.168.1.196]:')
    if not ip:
        ip = '192.168.1.196'  # Use a default IP address if no input is provided

# Connect to the xArm robot using the specified IP address
arm = XArmAPI(ip)  # Create an instance of the XArmAPI class with the IP address
time.sleep(0.5)  # Wait for a brief period to allow time for the connection to establish

# Clean any existing warnings or errors in the robot's status
if arm.warn_code != 0:
    arm.clean_warn()  # Clear any existing warnings
if arm.error_code != 0:
    arm.clean_error()  # Clear any existing errors

# Enable motion and set the robot's mode and state to 0
arm.motion_enable(True)  # Enable motion on the xArm
arm.set_mode(0)  # Set the mode of the xArm to 0 (position control mode)
arm.set_state(0)  # Set the state of the xArm to 0 (ready state)

# Set the tool GPIO modbus baudrate to 2000000
code = arm.set_tgpio_modbus_baudrate(2000000)  # Set the baudrate for the tool GPIO modbus communication
print('set_tgpio_modbus_baudrate, code={}'.format(code))  # Print the return code of the set_tgpio_modbus_baudrate command
time.sleep(2)  # Wait for 2 seconds

# Enable the bio gripper
code, ret = arm.getset_tgpio_modbus_data([0x08, 0x06, 0x01, 0x00, 0x00, 0x01])  # Send a modbus command to enable the bio gripper
print('set_bio_gripper_enable, code={}, ret={}'.format(code, ret))  # Print the return code and response of the enable command

# Set the bio gripper speed to 1000
speed = 1000  # Define the desired speed for the bio gripper
code, ret = arm.getset_tgpio_modbus_data([0x08, 0x06, 0x03, 0x03, speed // 256 % 256, speed % 256])  # Send a modbus command to set the gripper speed
print('set_bio_gripper_speed, code={}, ret={}'.format(code, ret))  # Print the return code and response of the set speed command

# Control the bio gripper in a loop until the robot is connected and has no errors
while arm.connected and arm.error_code == 0:
    # Send a modbus command to open the bio gripper
    code, ret = arm.getset_tgpio_modbus_data([0x08, 0x10, 0x07, 0x00, 0x00, 0x02, 0x04, 0x0, 0x0, 0x0, 0x82])
    print('open_bio_gripper, code={}, ret={}'.format(code, ret))  # Print the return code and response of the open command

    # Wait for 2 seconds
    time.sleep(2)

    # Send a modbus command to close the bio gripper
    code, ret = arm.getset_tgpio_modbus_data([0x08, 0x10, 0x07, 0x00, 0x00, 0x02, 0x04, 0x0, 0x0, 0x0, 0x32])
    print('close_bio_gripper, code={}, ret={}'.format(code, ret))  # Print the return code and response of the close command
    time.sleep(2)
