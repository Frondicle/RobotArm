#Chris Morrey 1/25/2023
#newest version!!!!!
import sys
import time
#from urllib import response
#from unicodedata import digit
import numpy as np
import pandas as pd
import struct
import logging 
import serial
logging.basicConfig(filename='output.txt',level=logging.DEBUG)

sys.path.append("C:\\Users\\morr0289\\Documents\\Github\\RobotArm\\Python\\final\\armFinal\\arm_sdk4")
#sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

#from arm_sdk4.xarm.wrapper.xarm_api import XArmAPI

xlpath = "C:\\Users\\morr0289\\Documents\\Repositories\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShortATOM.xlsx"


ser1 = serial.Serial(
    port="COM19", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
stp = pd.read_excel(xlpath, header = 0, index_col = None, usecols = [1,2,3,4,5], converters={'line':bytes,'numSteps':bytes,'stepTime':float,'dir':bytes,'end':bytes})

'''stp = pd.read_excel(xlpath, header = 0, index_col = None, usecols = [1,2,3,4,5], dtype = {'line':int,'numSteps':int,'stepTime':float,'dir':int,'end':int})'''

'''move = pd.read_excel(xlpath, "1", index_col='index',usecols=['index','x','y','z','pitch','roll','yaw','fspeed'])

#xArmDefaultSpeed = 50

#goHome = [0,305,0,100,0,-180,0,xArmDefaultSpeed]

ip = '192.168.1.240'
arm = XArmAPI(ip)

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
    return mv'''

def getStep(frame,ln):
    data = frame.loc[frame.index[ln]]
    line = bytes(data['line'])
    numSteps=bytes(data['numSteps'])
    stepTime = bytearray(struct.pack("d", stepTime)) 
    stepTime=bytes(data['stepTime'])
    dir = bytes(data['dir'])
    end= str(data['end'])
    dframe = [line, numSteps, stepTime, dir, end,'\n']
    return dframe

def getLine():
    global ser1
    ln = ser1.read(2)
    print ('from getLine: ',ln)
    line = ln.decode('utf-8')
    return line

'''def getQueueState():
    data_frame = [0x07,0x03,0x00,0x04,0x00,0x01]
    mod = arm.getset_tgpio_modbus_data(data_frame,host_id=9)
    code,response = mod
    print('queue state: ',code,", ", response)
    return response
    
def mover(move):
    arm.get_err_warn_code(show=True, lang='en')
    arm.clean_warn()
    arm.clean_error()
    arm.set_state(state=0)
    #arm.set_mode(mode=0)
    print('move: ',move)
    arm.set_position(x=move[1], y=move[2], z=move[3], roll=move[5], pitch=move[4], yaw=move[6], speed=move[7], is_radian=False, wait=True)
    arm.clean_error()


def setArmReady(x):
    if x =='TRUE':
        arm.set_tgpio_digital(1,1)
        print ('setArmReady: arm ready')
    elif x =='FALSE':
        arm.set_tgpio_digital(1,0)
        print ('setArmReady: arm not ready')

def endAll(datas,lastMove):
    time.sleep(2)
    #pd.ExcelFile.close(xlpath)
    arm.reset(wait=True)
    arm.set_state(state=4)
    arm.disconnect()'''
    
line = 0
end = 0
while (end == 0):
    toSend = (getStep(stp,line))
    toSend.append ('\n')
    print('toSend: ',toSend)
    ser1.write(toSend)
    #next = getLine()
    end = int(toSend[4])
    line =+ 1
else:
    ser1.close()
    



'''TGPIO for air solenoid---------------------------------------------
value = 0
    code = arm.set_cgpio_digital_output_function(i, value)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))
# Reset: 255
for i in range(4, 8):
    code = arm.set_cgpio_digital_output_function(i, 255)
    print('set_cgpio_digital_output_function({}, {}), code={}'.format(i, value, code))'''
'''
arm.reset(wait=True)
arm.motion_enable(enable=True)
arm.get_err_warn_code(show=True, lang='en')
arm.clean_warn()
arm.clean_error()
time.sleep(2)

speaker = win32com.client.Dispatch("SAPI.SpVoice")

speaker.speak("Resetting Extruder.")

arm.motion_enable(True)
arm.clean_error()
arm.set_mode(0)
arm.set_state(state = 0)
arm.reset(wait=True)
time.sleep(1)

id = 7
baudRate = 9600
ret = arm.core.set_modbus_timeout(7)  
print('set modbus timeout, ret = %d' % (ret[0]),'line 203')
#arm.clean_error()    
ret = arm.core.set_modbus_baudrate(baudRate)  
print('set modbus baudrate, ret = %d' % (ret[0]))
time.sleep(1)
#arm.set_state(state=0)
arm.set_tcp_load(weight=0.91, center_of_gravity=[0,80,-40]) #TCP info for extruder
arm.set_tcp_offset([0,0,111.5,0,-30.3,0], is_radian=False)
arm.set_collision_tool_model(22, x=40, y=40, z=111.5)
code, digitals = arm.get_tgpio_digital()

speaker.Speak("Robot Arm Connected.")

time.sleep(5)
#initial packet
sender(init)
print('init packet sent')
speaker.Speak("Initializing.")
time.sleep(1)
mv = getIndex()
while arm.connected:
        while mv <= mod.size / 5:
            print ('row: ',mv)
            arm.clean_error()
            #arm.motion_enable(True)
            setArmReady('TRUE')
            if (getExtruderFlag('ready') == 1):
                print('extruder ready') 
                mv = getIndex()
                print('mv: ',mv)
                datas = getMod(mod,mv)   
                #newMove = getMove(move,mv)
                sender(datas[0])
                mover(move)
            elif (getExtruderFlag('ready') == 0):
                time.sleep(.05)
#finish up:   endAll(end,goHome)'''
