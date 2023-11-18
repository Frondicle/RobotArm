import sys
import time
import numpy as np
import pandas as pd
sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")

mod = pd.read_excel("C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\grassData.xlsx", "1", index_col='index',usecols=['index','move','espeed','dir','end'],converters={'move':hex,'espeed':hex,'dir':hex,'end':hex})
draft=mod.loc[2]
print(draft)