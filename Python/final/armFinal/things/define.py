'''Chris Morrey 1/2/2023

some stuff

xArm state:return:  
1: in motion  
2: sleeping  
3: suspended  
4: stopping 

Note: Orientation of attitude angle
    roll: rotate around the X axis
    pitch: rotate around the Y axis
    yaw: rotate around the Z axis

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
0x00,0x00=register 5 current line
Digitals:
TI0: ready flag: 1=ready, 0=not ready
TI1: start flag:
TO0: reset extruder: 0=reset, 1=normal
TO1: 
'''
xArmIP = '192.168.1.240'
xArmSlaveID = 7
xArmBaudRate = 9600
xArmDefaultSpeed = 100

speak = 'TRUE'

xlpath = "C:\\Users\\morr0289\\Documents\\GitHub\\RobotArm\\Python\\final\\armFinal\\data_here\\grassDataWallsShort.xlsx"

init = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00] #clear all registers

end = [0x07,0x10,0x00,0x00,0x00,0x05,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01]#end of file flag to register 4

goHome = [305,0,100,0,-180,0,xArmDefaultSpeed]


  


