import serial
import time
import _winreg as winreg
import itertools
import datetime

def enumerate_serial_ports():
    """ Uses the Win32 registry to return an
        iterator of serial (COM) ports
        existing on this computer.
    """
    path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
    except WindowsError:
        raise IterationError

    for i in itertools.count():
        try:
            val = winreg.EnumValue(key, i)
            yield str(val[1])
        except EnvironmentError:
            break

connected = False

for porta in enumerate_serial_ports():
    print ("TRYING PORT: " + porta)
    start = datetime.datetime.now()
    delta=0
    ser = serial.Serial(porta,9600,timeout=0)
    if ser.isOpen():
        while ser.isOpen() and delta < 1:
            delta = (datetime.datetime.now()-start).seconds
            r = ser.read()
            if r == None:
                print ("connected!")