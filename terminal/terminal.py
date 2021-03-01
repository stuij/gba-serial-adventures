#!/usr/bin/env python

import argparse
import os
import serial
import sys

ser = None

def init(port, baudrate, rtscts):
    global ser
    ser = serial.Serial(port, baudrate, timeout=1, rtscts=rtscts)

def header_loop():
    cmd = input("> ")
    to_ser = cmd.encode('ascii', 'ignore') + b'\n'
    ser.write(bytearray([len(to_ser)]) + to_ser)
    while True:
        read = ser.read(max(1, ser.inWaiting())).decode('ascii')
        if read:
            if read == 'A':
                print("OK")
                break
            else:
                print("didn't get OK signal: {0}".format(read))
                break

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
    parser.add_argument('--protocol', dest="header", action='store_true',
                        help="use header protocol, instead pass-through of data")

    args = parser.parse_args()

    init(args.port, args.baudrate, args.rtscts)

    print("starting terminal in {0} mode".format("protocol" if args.header else "passthrough"))
    read_loop(header_loop if args.header else passthrough_loop)


if __name__ == "__main__":
    main()
