import os  # Provides functions for interacting with the operating system
import sys  # Provides access to some variables used or maintained by the interpreter
import time  # Provides various time-related functions
sys.path.append(os.path.join(os.path.dirname(__file__)'C:\Users\morr0289\Documents\GitHub\RobotArm\Python\xArm-Python-SDK-master'))

from xarm.wrapper import XArmAPI  # Import the xArmAPI from the xarm.wrapper module
from configparser import ConfigParser  # Import the ConfigParser class from the configparser module

print(xarm.wrapper.path)
print(configparser.path)