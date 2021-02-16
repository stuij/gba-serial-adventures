#!/usr/bin/env python

import argparse
import os
import serial
import sys
import threading

ser = None

def init(port, baudrate, rtscts):
    global ser
    ser = serial.Serial(port, baudrate, timeout=None, rtscts=rtscts)
    out = threading.Thread(target = write_loop)
    out.start()

def read_loop():
    while True:
        try:
            cmd = input()
            ser.write(cmd.encode('ascii', 'ignore') + b'\n')
        except KeyboardInterrupt:
            try:
                sys.exit(0)
            except SystemExit:
                print("\nexiting..")
                os._exit(0)

def write_loop():
    while True:
        sys.stdout.buffer.write(ser.read())
        sys.stdout.flush()

def main():
    parser = argparse.ArgumentParser(description='serial repl')
    parser.add_argument('port',
                        help='The name of the serial port. examples: `/dev/ttyUSB1`,  `COM3`')
    parser.add_argument('-b', '--baudrate', type=int, default=115200,
                        choices=[9600, 38400, 57600, 115200],
                        help='the baud-rate of the connection')
    parser.add_argument('--no-rtscts', dest="rtscts", action='store_false',
                        help="don't use RTS/CTS hardware flow control")

    args = parser.parse_args()

    init(args.port, args.baudrate, args.rtscts)
    read_loop()


if __name__ == "__main__":
    main()
