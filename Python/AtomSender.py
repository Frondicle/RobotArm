'''uFactory xArm6 controller with external AtomS3 control for WASP extruder stepper
Chris Morrey 4/2024'''
import serial
import time
import asyncio

ser1 = serial.Serial(
    port="COM4", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
