(I'm hedging my bets with this repo name. I'd want to add more serial setup tutorials. For example, I bought a bunch of Gameboy Advance wireless adaptors that I should do something with. It's just a GBA UART demo so far though.)

# Gameboy Advance UART demo

It turns out you don't need an Xboo, MBV2, Arduino, own microchip etc.. setup to
communicate with a Gameboy Advance over the link port. A run-of-the-mill USB-to-UART cable
will do fine. And as an interface program any old UART app will probably do as
well.

This setup can also be used to load multiboot programs.

This is a tutorial and simple code example to test UART transfers with this setup. An option to load multiboot programs is included.


![GBA UART demo](uart.jpeg?raw=true "See, it works.")

I've tested this in two setups:

###  5-wire setup

RX (receive), TX (transmit), both the RTS and CTS control flow lines and ground

This enables full flow control, but requires link cable and USB<->UART cable to
have all the flow control lines available. Most GBA and USB<->UART cables don't have
all the wires. However GB cables will fit the GBA and (I think) always have all six.

### 3-wire setup

At a minimum you only need RX, TX and ground on both cables. So no flow
control.

## what setups will work

As it turns out you don't need full flow control to get communication going. The
standard USB to serial cables that are often bought to interface with your
Raspberry Pi will only have power, ground, RX and TX. With those you can
communicate just fine.

However, since the GBA can't tell the other side to stop sending, it won't. This
should be an issue when the GBA can't process all the incoming bytes in time for
whatever reason.

The GBA can be set up with or without hardware flow control, by setting the
SIO_CTS bit in the SIOCNT register.

On my setup, with SIO_CTS set, individual transfers will still work two-way with
the brown RTS cable disconnected. When pulling the green CTS cable, only
transfers to the GBA will go through. See below for how to wire things up. But
this will depend on how your client UART program behaves as well.

With SIO_CTS unset, you only need RX, TX and ground.

I believe/hope the code example should work with either setup.

## what do you need

### USB to UART cable

Again, just be aware what cable you're buying. Most of them exclude the RTS and
CTS wires, and so will only expose 4 wires.

I went with the FDTI TTL-232R-3V3. You can choose between female end-points or
just wires. The female endpoints can be handy if you want to debug the protocol
as you can plug them on a breadboard.

### Link cable

Again, if you're going for the full setup, you need a link cable wit at least 5
wires actually connected to the connector pins. Most aftermarket cables don't,
so be ware. I've bought a few standard Japanese link cables from Ebay, and they
do have five pins connected. Those Japanese link cables do miss the VCC pin, but
it's not necessary for this purpose.

The best way to check is to see if there are metal plates on all of the
outputs. It's usually hard to see on the pictures.

### terminal program

I've wrote a simple Python terminal that is hopefully cross-platform:

    <root>/shell/shell.py -h

Whatever you type is sent over the specified serial port after you hit
return. The program asynchronously prints whatever the GBA sends back on the
screen. Read below for more info on its features and operation.

You can also choose your favorite terminal program. GNU screen should work on Linux. Make sure you enable serial flow control if your cable (and the GBA program) supports it. If you use it with the demo program in this repo. Press left on the dpad to set the GBA into pass-through mode, which is compatible with dumb terminals.

### compiler toolchain

DevkitPro works for me

### tonc library

for writing text to screen

## hardware setup

It's just a matter of wiring up the USB cable to the GBA link cable. In my debugging setup, I put male pins on the link cable wires. They plug either into the female end-points of the UART cable or into a breadboard.

The below schematics should hopefully make it obvious how to wire up both cables:

### GBA link-port pinout

     _______
    / 1 3 5 \
    | 2 4 6 |	(looking at GBA cable link-port, so not GBA itself)
    '-------'

(colors below are taken from original Japanese blue link cable)

    1	VCC xxx
    2	SO  TX  red
    3	SI  RX  orange
    4	SD  RTS brown
    5	SC  CTS green
    6	GND GND blue


### FDTI TTL-232R-3V3 cable pinouts
(https://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232R_CABLES.pdf)

    1 GND black
    2 CTS brown
    3 VCC red
    4 TDX orange
    5 RDX yellow
    6 RTS green


### GBA <-> rs232 usb cable wiring

    GBA                rs232 usb cable  oscilloscope
    ---                ---------------  ------------
    2 SO TX  red ----> 5 RxD yel        purple
    3 SI RX  org <---- 4 TxD org        blue
    4 SD RTS bwn ----> 2 CTS bwn        green
    5 SC CTS grn <---- 6 RTS grn        yellow
    6 GND blu <------> 1 GND blk

At a minimum you can get by with GND <-> GND, SO <-> RxD, SI <-> TxD.

![what cable wiring looks like](uart-cable.jpeg?raw=true "My setup. I should make another one, with the cables fused.")

## compiling

Check the Makefile to see if the top 3 parameters work for you:

- PATH - path to your toolchain binaries
- TONCLIB - path to tonclib
- FLOW_CONTROL - flow control on or off

Type `make` in the root of the repo.

## operation

On the GBA the baud is set to 115200, so your communication program must follow
suit. There's no handshaking protocol whatsoever. You can disconnect/reconnect
your GBA or terminal program on your other computer at any time, and just type
away when both are set up.

steps (in any order):
- connect frankencable to GBA and terminal computer
- start the compiled rom on your GBA
- On the terminal computer, check what port it's on. On Linux do a diff between
  two invocations of `ls /dev/tty*` with and without the cable plugged in. It's
  often on /dev/ttyUSB0. Not sure how to find out on Windows, but I believe it
  should be `COMx`, where `x` is a number.
- start your favorite serial communication program or use the simplistic
  terminal program in this repo: `<root>/shell/shell.py /dev/ttyUSB1`
- When using a dumb terminal, press left on the GBA dpad to enter dumb
  pass-through mode.
- (the shell.py program will only send data to the GBA after a `return`. The
  GBA program will only exit the read loop after detecting a `return`. The GBA
  will send back the bytes you've sent it)
- (the read-buffer of the GBA program is 4096 bytes)

## protocols and terminal program

The GBA test program and the included shell.py program supports two protocols:

- passthrough: this will just read incoming bytes into an array, scan for a ret
  character and then print those bytes to screen.

- Gbaser (the default): This is a very simple communication protocol. The first
  byte designates the type, then follows a word containing the size of the
  transfer, then the data itself, and following it another word with the CRC of
  the data. The terminal program by default will listen to strings of data, and
  will send whatever you typed in the Gbaser format, with as type `string'. On
  the GBA, the result will be saved in a ring-buffer, which will be drained when
  printing the characters on screen.

## sending multiboot programs

You can also send and execute multiboot programs, so GBA roms with the `.mb` suffix.

Just use the `shell.py` program with as the argument of the `--multiboot`
parameter your multiboot file:

    ./shell/shell.py --multiboot ~/iso-snake.mb /dev/ttyUSB0

## sending binary blobs

You can also send binary blobs to any GBA memory location. In this way you can
for example load bitmaps, sprites, palettes or tiled backgrounds interactively
from your computer terminal, instead of having to compile and load your whole
program to say a flash cart every time you make a change.

To do this you can for example use Grit to create a palette and data file from
your image, and send them to the GBA one after the other with the shell. The
below invocation is equivalent to the `--multiboot` example above, except that
the GBA won't branch to 0x02000000 (it's a different Gbaser type):

    ../rath/shell/shell.py --binary-blob /home/zeno/tmp/iso-snake.mb --binary-loc 0x02000000 /dev/ttyUSB0

## gotchas

It looks like the computer-side takes a byte to respect SD being high. I'm not
sure what component is the culprit. Perhaps it's the FDTI chip in the cable,
perhaps it's the serial tty setting in the OS. I do see this behaviour both in
the Emacs UART client as well as with PySerial used by the Python terminal app.

## CPU budget per UART char received

Some perspective on how many instructions can be processed in the time it takes
one byte to send: At a baud rate of 115200, it takes 8.68 usec for 1 bit to
transfer. So 8 bits plus one start and one stop bit will take 86.8 usec to
transfer. The CPU frequency of the GBA is 16.78 Mhz, or 59.59 nsec per
instruction. So running at UART line speed, we have 1,456.62 of instructions to
play with between UART data register reads. A screen refresh takes 280,896
cycles, so a byte read is about 0.005 of that, or 1/192.84. So 0.84 scanlines
(including vblank).

## todo

- add oscilloscope pics
- investigate why the computer-side doesn't respect SD being high
- set a read timeout

## acknowledgement

- The UART-specific code was initially copied from Adrian O'Grady:
  https://web.archive.org/web/20050425075428/http://www.fivemouse.com/gba/
  Unfortunately it turns out that code was quite uninformed, so It'd be unwise
  to use it as an example. But it was a great starting point.
- The console-like text behaviour was ripped from pandaforth:
  https://github.com/iansharkey/pandaforth

## license

This repo is distributed under the MIT license. For licensing terms, see the
LICENSE file in the root of this repo or go to
http://opensource.org/licenses/MIT
