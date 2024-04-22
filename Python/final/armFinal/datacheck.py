import sys
import time
#from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import logging 
logging.basicConfig(filename='output.txt',level=logging.DEBUG)

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShortATOM.xlsx"

stp = pd.read_excel(xlpath, header = 0, index_col = None, usecols = [1,2,3,4,5], dtype = {'line':int,'numSteps':int,'stepTime':float,'dir':int,'end':int})

print (stp.head(3))
