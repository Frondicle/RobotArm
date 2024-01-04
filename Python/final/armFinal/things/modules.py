'''Chris Morrey 1/2/2024
modules for robot arm control

Modbus format:
0x07=slave id
0x10=function 16 write registers, function 3 read registers
0x00,0x00=beginning address to write
0x00,0x05 = number of registers
0x0a=number of bytes(2x registers)
0x00,0x15=register 0 index
0x00,0x06=register 1 steps
0x00,0x00=register 2 espeed
0x00,0x01=register 3 direction 1=forward 0=reverse
0x00,0x00=register 4 end of file 1=stop 0=continue
0x00,0x00=register 5 spare
Digitals:
TI0: ready flag: 1=ready, 0=not ready
TI1: start flag:
TO0: reset extruder: 0=reset, 1=normal
TO1: 
'''

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\")
import sys
import time
from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd 
from ..arm_sdk3.xarm.wrapper.xarm_api import XArmAPI
import define

#Initialize arm stuff--------------------------------------------
def armStart():
    arm = XArmAPI(xArmIP,is_radian=False)
    id = xArmSlaveID
    baudRate = xArmBaudRate
    ret = arm.core.set_modbus_timeout(7)  
    print('set modbus timeout, ret = %d' % (ret[0]))    
    ret = arm.core.set_modbus_baudrate(baudRate)  
    print('set modbus baudrate, ret = %d' % (ret[0]))
    time.sleep(1)
    code, digitals = arm.get_tgpio_digital()
    if arm.warn_code != 0:
        arm.clean_warn()
    if arm.error_code != 0:
        arm.clean_error()
    time.sleep(2)

    arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
    arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
    arm.set_collision_tool_model(22, x=40, y=40, z=111.5)


#Excel file reader------------------------------------------------
def makeFrames():
    mod = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','steps','espeed','dir','end'])
    move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])
    return mod,move

#Error interpreter-------------------------------------------------
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

#xArm movement data frame interpreter----------------------------------
def getMove(frame,ln):
    move = frame.iloc[ln]
    pti = move['index']
    ptx = move['x']
    pty = move['y']
    ptz = move['z']
    ptp = move['pitch']
    ptr = move['roll']
    ptyaw = move['yaw']
    pts = move['fspeed']
    mv = [pti,ptx,pty,ptz,ptp,ptr,ptyaw,pts]
    return mv
#Extruder data frame interpreter--------------------------------------
def modFrame(frame,ln):
    modData=frame.iloc[ln]
    indx = modData['index']
    stps=modData['steps']
    spd=modData['espeed']
    dir=modData['dir']
    end=modData['end']
    dframe = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a, indx, stps, spd, dir, end]
    response = (dframe,indx)
    return response

#Modbus sender------------------------------------------------------
def sender(data_frame):
    global id
    code, digitals = arm.get_tgpio_digital()
    #queueState = getQueueState
    #print ('queue state: ',queueState)
    #while queueState == 1 and code != 19 and code != 28:
    #while arm.connected and arm.error_code != 19 and arm.error_code != 28:
    ret = arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    if ret[0] == 0:
        print('sender: data sent')
    else:
        print('sender: ',ret[0])

def mover(move):
    arm.get_err_warn_code(show=True, lang='en')
    arm.clean_warn()
    arm.clean_error()
    arm.set_state(state=0)
    arm.set_mode(mode=0)
    print(move)
    arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=move[7], is_radian=False, wait=True)

if __name__ == '__main__':
    sys.exit(main())  # next section explains the use of sys.exit


'''TGPIO for air solenoid---------------------------------------------
value = 0
    code = arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''

sender(init)
print('init packet sent')
speaker.Speak("Initializing.")
line = 0
while arm.connected:
    while line <= mod.size / 11:
        print ('line: ',line)
        arm.clean_error()
        if (arm.get_tgpio_digital(0) == 1):    
            arm.set_state(state = 0)
            #print('ready flag ',arm.get_tgpio_digital(0))
            datas = modFrame(mod,line)   
            newMove = getMove(move,datas[1])
            arm.set_state(state=1)
            sender(datas[0])
            mover(newMove)
        line += 1
        #else:
            #time.sleep(.1)
#finish up:   
datas =[0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]
sender(datas)
mover([305,0,100,0,-180,0,armspeed])#x=move[0], y=move[1], z=move[2], roll=move[4], pitch=move[3], yaw=move[5], speed=move[6]
time.sleep(2)
pd.ExcelFile.close(xlpath)
arm.reset(wait=True)
arm.disconnect()
