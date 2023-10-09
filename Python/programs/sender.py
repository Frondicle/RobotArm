def sender(data_frame):
    global line
    global id
    arm.clean_error()
    arm.set_state(state=0)
    code, digitals = arm.get_tgpio_digital()
    #while arm.connected and arm.error_code != 19 and arm.error_code != 28:
    if code == 0 and digitals[0] == 1:
        print ('extruder reports ready')
        ret = arm.getset_tgpio_modbus_data(data_frame,host_id=id)
        if ret[0] == 0:
            print('wrote ',data_frame[8])
            print('line: ',line)
            line += 3
            time.sleep(.75)
    elif digitals [0] == 0:
        print('extruder reports not ready')
    elif code != 0:
        errors(ret[0])

def buildFrame(filedata,ln):
    moves = filedata.read(ln)
    frame = ('0x09,0x10,0x00,0x00,0x00,0x03,0x06,',moves)
    return frame

def errors(returned):
    if returned == 1:
        print('1: uncleared error exists')
    elif returned == 23:
        print('23:modbus reply length error')
    elif returned == 20:
        print('20: wrong slave id')
    elif returned == 15:
        print('15: Modbus commands full')
    elif returned == 19:
        print('19: communication error')
    elif returned == 28:
        print('28: end module communication error')
    else:
        print('other error: ',returned)
    input=('clear errors? (y/n)')
    if input == 'y':
            arm.clean_error()
    else:
        print('error not cleared; quitting...')
        time.sleep(2)
        quit()


