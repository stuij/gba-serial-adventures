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
    string = 0x00
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
    return struct.pack('<IB{0}sI'.format(length), length, kind, data, crc)

def gbaser_loop():
    cmd = input("> ")
    ascii = cmd.encode('ascii', 'ignore') + b'\n'
    msg_type = Mtype.string.value # string
    to_ser = make_msg(msg_type, ascii)
    ser.write(to_ser)

    reply = b''
    data_len = None

    while True:
        read = ser.read(max(1, ser.inWaiting()))
        if len(read) >= 1:
            reply += read

        if len(reply) >= 4 and data_len == None:
            data_len = int.from_bytes(reply[:4], 'little', signed=True)

        if data_len != None and len(reply) >= data_len + 9:
            break

    msg_type = reply[4]
    data = reply[5:5 + data_len]
    our_crc = zlib.crc32(data)
    their_crc = int.from_bytes(reply[-4:], 'little', signed=False)

    if our_crc == their_crc:
        print("OK")
    else:
        print("ERROR: CRCs don't match")
        print(reply)
        print("data_len: '{:08x}', type: {:x}".format(data_len, msg_type))
        print("data: '{0}'".format(data))
        print("CRCs - ours: '{:08x}', theirs: '{:08x}'".format(our_crc, their_crc))


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
    read = ser.read(ser.inWaiting()).decode('ascii')
    print("read garbage: `{0}`".format(read))
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
    parser.add_argument('--gbaser', dest="gbaser", action='store_true',
                        help="use gbaser protocol, instead pass-through of data")

    args = parser.parse_args()

    init(args.port, args.baudrate, args.rtscts)

    print("starting terminal in {0} mode".format("protocol" if args.gbaser else "passthrough"))
    read_loop(gbaser_loop if args.gbaser else passthrough_loop)


if __name__ == "__main__":
    main()
