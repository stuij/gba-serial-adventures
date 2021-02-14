import serial

# TODO: this should be an actual repl. Not just fns executed on a Python repl.
ser = serial.Serial("/dev/ttyUSB1", 115200, timeout=None, rtscts=1)

def print_string(string):
    ser.write(bytearray([len(string)]) + string)

