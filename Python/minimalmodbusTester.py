import minimalmodbus

PORT='COM6'
#PORT='/dev/ttyUSB0'

#Set up instrument
instrument = minimalmodbus.Instrument(PORT,9,mode=minimalmodbus.MODE_RTU)

#Make the settings explicit
instrument.serial.baudrate = 38400        # Baud
instrument.serial.bytesize = 8
instrument.serial.parity   = minimalmodbus.serial.PARITY_NONE
instrument.serial.stopbits = 1
instrument.serial.timeout  = 1          # seconds
instrument.address = 9      # this is the slave address number
instrument.close_port_after_each_call = True
instrument.clear_buffers_before_each_transaction = True

# Read temperatureas a float
# if you need to read a 16 bit register use instrument.read_register()
instrument.write_register(0,23,1)
