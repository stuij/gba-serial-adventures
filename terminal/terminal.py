#!/usr/bin/env python

# Gbaser message format (little endian):
# len:    4 bytes, length of bytes in data field
# type:    1 byte, type of message
# data: len bytes, actual data
# crc:    4 bytes, crc check
#
# types:
# - 0x00: string
# - 0xFF: return OK
# - 0xFE: general error
# - 0xFD: CRC error

import argparse
from enum import Enum
import os
import serial
import struct
import sys
import zlib

class Mtype(Enum):
    undefined = 0x00
    string = 0x01
    multiboot = 0x02
    ret_ok = 0xff
    ret_error = 0xfe
    ret_crc_error = 0xfd

ser = None

def init(port, baudrate, rtscts):
    global ser
    ser = serial.Serial(port, baudrate, timeout=1, rtscts=rtscts)

def make_msg(kind, data):
    length = len(data)
    crc = zlib.crc32(data)
    return struct.pack('<Bi{0}sI'.format(length), kind, length, data, crc)

def send_gbaser_string(cmd):
    ascii = cmd.encode('ascii', 'ignore') + b'\n'
    msg_type = Mtype.string.value # string
    print("msg length: {0}".format(len(ascii)))
    to_ser = make_msg(msg_type, ascii)
    print("to ser: {0}".format(to_ser))
    ser.write(to_ser)

def send_multiboot(file):
    with open(file, 'rb') as fp:
        bytes = bytearray(fp.read())
        msg_type = Mtype.multiboot.value # string
        print("sending multiboot rom, please wait..")
        print("file length: {0}".format(len(bytes)))
        to_ser = make_msg(msg_type, bytes)
        # print("to ser: {0}".format(to_ser))
        ser.write(to_ser)

def get_gbaser_reply():
    reply = b''
    data_len = None

    while True:
        read = ser.read(max(1, ser.inWaiting()))

        # we might have timed out and therefore have read nothing
        if len(read) >= 1:
            reply += read

        # remove gba startup garbage. This can be either b'\x00\xff' or
        # b'\x00\x00\x00\x00\x00\xff' depending on if fifo is activated.  To be
        # a bit succinct with the code, we're iterating over the reply if
        # necessary, counting on the 1 second read timeout. (there's no way
        # telling how much code might have been received, but removing in pairs
        # should do what we want concidering the above patterns, even if we get
        # odd amounts of data in)
        if len(reply) >= 5 and reply[:2] in [b'\x00\xff', b'\x00\x00']:
            print("deleting gba startup gunk: {0}".format(reply[:2]))
            reply = reply[2:]

        if (len(reply) >= 5 and data_len == None
            and reply[:2] not in [b'\x00\xff', b'\x00\x00']):
            data_len = int.from_bytes(reply[1:5], 'little', signed=True)

        if data_len != None and len(reply) >= data_len + 9:
            break

    msg_type = reply[0]
    crc_begin = 5 + data_len
    data = reply[5:crc_begin]
    our_crc = zlib.crc32(data)
    their_crc = int.from_bytes(reply[crc_begin:crc_begin + 4], 'little', signed=False)

    if our_crc == their_crc:
        print("got reply")
    else:
        print("ERROR: CRCs don't match")
        print(reply)
        print("data_len: '{:08x}', type: {:x}".format(data_len, msg_type))
        print("data: '{0}'".format(data))
        print("CRCs - ours: '{:08x}', theirs: '{:08x}'".format(our_crc, their_crc))

def gbaser_loop():
    cmd = input("> ")
    if (cmd.startswith("multiboot")):
        send_multiboot(cmd.split(" ")[1].strip())
    else:
        send_gbaser_string(cmd)

    get_gbaser_reply()

def passthrough_loop():
    cmd = input("> ")
    to_ser = cmd.encode('ascii', 'ignore') + b'\n'
    ser.write(to_ser)
    cmd_len = len(cmd)

    reply = ''
    while True:
        read = ser.read(max(1, ser.inWaiting())).decode('ascii')
        if len(read) >= 1:
            reply += read
            if len(reply) >= cmd_len:
                print(reply, end='')
                break

def read_loop(fn):
    while True:
        try:
            fn()
        except KeyboardInterrupt:
            try:
                sys.exit(0)
            except SystemExit:
                print("\nexiting..")
                os._exit(0)

def main():
    parser = argparse.ArgumentParser(description='serial repl')
    parser.add_argument('port',
                        help='The name of the serial port. examples: `/dev/ttyUSB1`,  `COM3`')
    parser.add_argument('-b', '--baudrate', type=int, default=115200,
                        choices=[9600, 38400, 57600, 115200],
                        help='the baud-rate of the connection')
    parser.add_argument('--no-rtscts', dest="rtscts", action='store_false',
                        help="don't use RTS/CTS hardware flow control")
    parser.add_argument('--passthrough', dest="passthrough", action='store_true',
                        help="use pass-through mode of sending data, instead of Gbaser")
    parser.add_argument('-m', '--multiboot', default='', help="load this multiboot file")

    args = parser.parse_args()

    init(args.port, args.baudrate, args.rtscts)

    if(args.multiboot):
        send_multiboot(args.multiboot)
    else:
        print("starting terminal in {0} mode".format(
            "pass-through" if args.passthrough else "Gbaser"))
        read_loop(passthrough_loop if args.passthrough else gbaser_loop)


if __name__ == "__main__":
    main()
