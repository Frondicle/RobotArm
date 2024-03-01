import minimalmodbus
import serial

PORT='COM6' #be sure your port settings in Windows correspond to the settings here; you can check in Settings, search for Device Manager. If you're not sure which port, use serialportchecker.py, hopefully in this same folder.


#Set up instrument
instrument = minimalmodbus.Instrument(PORT,7,minimalmodbus.MODE_RTU)

#Make the settings explicit
instrument.serial.baudrate = 9600        # 9600 is the baudrate the arduino is configured to us
instrument.serial.bytesize = 8
instrument.serial.parity   = minimalmodbus.serial.PARITY_NONE
instrument.serial.stopbits = 1
instrument.serial.timeout  = 1          # seconds
instrument.address = 7      # this is the slave address number arduino is configured to use.    
instrument.close_port_after_each_call = True
instrument.clear_buffers_before_each_transaction = True

try:
    ser = serial.Serial(PORT, instrument.serial.baudrate)
except serial.serialutil.SerialException:
    print ("Arduino not connected")

# if you need to read a 16 bit register use instrument.read_register()
instrument.write_register(0x00,0,0)
instrument.write_register(0x01,45,0)
instrument.write_register(0x02,0,0)
instrument.write_register(0x03,12,0)
instrument.write_register(0x04,0,0)
#instrument.write_register(0x05,0,0)
