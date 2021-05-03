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

# GBA defs for asset test/remote programming demo:
REG_BASE = 0x04000000
REG_DISPCNT = REG_BASE + 0x0000
REG_BG0CNT = REG_BASE + 0x0008

# mode3 demo
DCNT_MODE3 = 0x0003
DCNT_BG2 = 0x0400

MEM_VRAM = 0x06000000
MEM_VRAM_OBJ = 0x06010000
MEM_PAL	= 0x05000000
MEM_PAL_OBJ = 0x05000200

# obj demo + tiling bg demo
DCNT_OBJ = 0x1000
DCNT_OBJ_1D = 0x0040

# set up BG0 for a 4bpp 64x32t map, using
# using charblock 0 and screenblock 31
# REG_BG0CNT= BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_REG_64x32;
# REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0 | DCNT_BG0;
DCNT_MODE0 = 0x0000
DCNT_BG0 = 0x0100
BG_4BPP = 0
BG_8BPP	= 0x0080
BG_REG_64x32 = 0x4000
SCRBLK_SIZE = 0x800

BG_CBB_SHIFT = 2
def BG_CBB(n):
    return n << BG_CBB_SHIFT

BG_SBB_SHIFT = 8
def BG_SBB(n):
    return n << BG_SBB_SHIFT


class Mtype(Enum):
    undefined = 0x00
    string = 0x01
    binary = 0x02
    multiboot = 0x03
    ret_ok = 0xff
    ret_error = 0xfe
    ret_crc_error = 0xfd

SER = None
RATH = False

def init(port, baudrate, rtscts):
    global SER
    SER = serial.Serial(port, baudrate, timeout=1, rtscts=rtscts)
    print(SER)

def make_msg(kind, data):
    length = len(data)
    crc = zlib.crc32(data)
    return struct.pack('<Bi{0}sI'.format(length), kind, length, data, crc)

def send_gbaser_string(cmd):
    ascii = cmd.encode('ascii', 'ignore') + b'\r\n'
    msg_type = Mtype.string.value # string
    # print("msg length: {0}".format(len(ascii)))
    to_ser = make_msg(msg_type, ascii)
    # print("to ser: {0}".format(len(to_ser))
    SER.write(to_ser)


def get_gbaser_reply():
    reply = b''
    data_len = None

    while True:
        read = SER.read(max(1, SER.inWaiting()))

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
        pass # print("got reply: {0}".format(reply))
    else:
        print("ERROR: CRCs don't match")
        print(reply)
        print("data_len: '{:08x}', type: {:x}".format(data_len, msg_type))
        print("data: '{0}'".format(data))
        print("CRCs - ours: '{:08x}', theirs: '{:08x}'".format(our_crc, their_crc))

    rest = reply[crc_begin + 4:] # rest
    return type, rest


def print_remote_output(residue):
    reply = b''
    clean = ''
    exit = False

    def process_remote(remote):
        nonlocal reply, clean, exit
        clean = remote.replace(b'\x1e', b'')
        reply += clean

        # print(remote, clean, reply)
        sys.stdout.write(clean.decode('ascii', 'ignore'))
        sys.stdout.flush()
        if b'\x1e' in remote:
            exit = True

    process_remote(residue)

    while exit == False:
        read = SER.read(max(1, SER.inWaiting()))
        # we might have timed out and therefore have read nothing
        if len(read) >= 1:
            process_remote(read)


def send_binary(file, offset, msg_type):
    with open(file, 'rb') as fp:
        print(msg_type)
        bytes = bytearray(fp.read())
        send_bytes(bytes, offset, msg_type)


def send_bytes(bytes, offset, msg_type):
    offset_bytes = offset.to_bytes(4, 'little', signed=False)
    print("binary length: {0}".format(hex(len(bytes))))
    payload = offset_bytes + bytes
    print("total bytes: {0}".format(hex(len(payload))))
    to_ser = make_msg(msg_type, payload)
    print("to ser: {0}".format(hex(len(to_ser))))
    SER.write(to_ser)
    get_gbaser_reply()


def set_reg(nr, reg):
    val = nr.to_bytes(2, 'little', signed=False)
    send_bytes(val, reg, Mtype.binary.value)


def set_mode3_bg(file):
    set_reg(DCNT_MODE3 | DCNT_BG2, REG_DISPCNT)
    send_binary(file, MEM_VRAM, Mtype.binary.value)


def set_tile_bg(prefix_path):
    set_reg(DCNT_OBJ | DCNT_OBJ_1D | BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_64x32, REG_BG0CNT)
    set_reg(DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0 | DCNT_BG0, REG_DISPCNT)
    send_binary(prefix_path + '.img.bin', MEM_VRAM, Mtype.binary.value)
    send_binary(prefix_path + '.pal.bin', MEM_PAL, Mtype.binary.value)
    send_binary(prefix_path + '.map.bin', MEM_VRAM + SCRBLK_SIZE * 30, Mtype.binary.value)


def set_sprite_gfx(prefix_path):
    send_binary(prefix_path + '.img.bin', MEM_VRAM_OBJ, Mtype.binary.value)
    send_binary(prefix_path + '.pal.bin', MEM_PAL_OBJ, Mtype.binary.value)


def send_line(line):
    send_gbaser_string(line)
    msg_type, rest = get_gbaser_reply()
    if RATH:
        print_remote_output(rest)


def send_file(file):
    with open(file) as fp:
        for line in fp:
            send_line(line)

def send_multiboot(file):
    print("sending multiboot rom, please wait..")
    send_binary(file, 0x02000000, Mtype.multiboot.value)

def gbaser_loop():
    if RATH:
        cmd = input("")
    else:
        cmd = input("> ")

    if (cmd.startswith("multiboot")):
        send_multiboot(cmd.split(" ")[1].strip())
    elif (cmd.startswith("mode3-bg")):
        set_mode3_bg(cmd.split(" ")[1].strip())
    elif (cmd.startswith("tile-bg")):
        set_tile_bg(cmd.split(" ")[1].strip())
    elif (cmd.startswith("sprite-gfx")):
        set_sprite_gfx(cmd.split(" ")[1].strip())
    elif (cmd.startswith("binary")):
        arguments = cmd.split(" ")
        send_binary(arguments[1].strip(), int(arguments[2].strip()), Mtype.binary.value)
    elif (cmd.startswith("include")):
        send_file(cmd.split(" ")[1].strip())
    else:
        send_line(cmd)


def passthrough_loop():
    cmd = input("> ")
    to_ser = cmd.encode('ascii', 'ignore') + b'\n'
    SER.write(to_ser)
    cmd_len = len(cmd)

    reply = ''
    while True:
        read = SER.read(max(1, SER.inWaiting())).decode('ascii')
        if len(read) >= 1:
            reply += read
            if len(reply) >= cmd_len:
                print(reply, end='')
                break


def read_loop(fn):
    read = SER.read(max(0, SER.inWaiting()))
    if len(read) > 0:
        sys.stdout.write(read.decode('ascii', 'ignore') + "\n")
        sys.stdout.flush()

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
    global RATH
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
    parser.add_argument('--rath', dest="rath", action='store_true',
                        help="Communicate with Rath Forth process. Implies Gbaser. Changes cmdline to be more Forth-like, and processes Forth output next to ack return for sending data.")
    parser.add_argument('-m', '--multiboot', default='', help="load this multiboot file")
    parser.add_argument('--binary-blob', dest="bin_blob",
                        help="binary blob to send to GBA")
    parser.add_argument('--binary-loc', dest="bin_loc",
                        help="location to send the binary blob to")
    parser.add_argument('--mode3-bg', dest="mode3_bg",
                        help="set mode3 background to 240x160 raw file")
    parser.add_argument('--tile-bg', dest="tile_bg",
                        help="set tiles, palette data and tile map of mode0 bg from common prefix of files in same dir. ex: `assets/brin`")
    parser.add_argument('--sprite-gfx', dest="sprite_gfx",
                        help="set tiles and palette data of sprite already initialized from common prefix of files in same dir. ex: `assets/ramio`")

    args = parser.parse_args()
    RATH = args.rath
    init(args.port, args.baudrate, args.rtscts)

    if(args.multiboot):
        send_multiboot(args.multiboot)
    elif(args.mode3_bg):
        set_mode3_bg(args.mode3_bg)
    elif(args.tile_bg):
        set_tile_bg(args.tile_bg)
    elif(args.sprite_gfx):
        set_sprite_gfx(args.sprite_gfx)
    elif(args.bin_blob):
        if not args.bin_loc:
            print("when sending a binary blob, you need to specify the memory location with `--binary-loc`")
        else:
            print(hex(int(args.bin_loc, 0)))
            send_binary(args.bin_blob, int(args.bin_loc, 0), Mtype.binary.value)
    else:
        print("starting terminal in {0} mode".format("passthrough" if args.passthrough else "Gbaser"))
        read_loop(passthrough_loop if args.passthrough else gbaser_loop)


if __name__ == "__main__":
    main()
