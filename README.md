I'm hedging my bets with this repo name. I'd want to add more serial setup tutorials. For example, I bought a bunch of Gameboy Advance wireless adaptors that I should do something with. It's just a GBA UART demo so far though.

# Gameboy Advance UART demo

It turns out you don't need an Xboo, MBV2, Arduino, own microchip etc.. setup to
communicate with a Gameboy Advance over the link port. A run-of-the-mill USB-to-UART cable
will do fine. And as an interface program any old UART app will probably do as
well.

This is a tutorial and simple code example to test UART transfers with this setup.

![GBA UART demo](uart.jpeg?raw=true "See, it works.")

I've tested this in two setups:

###  5-wire setup

RX (receive), TX (transmit), both the RTS and CTS control flow lines and ground

This enables full flow control, but requires link cable and USB<->UART cable to
have all the flow control lines available. Most cables of both kinds don't have
all the wires.

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

Choose your favorite terminal program. I'm using GNU screen on Linux.

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

    GBA                rs232 usb cable
    ---                ---------------
    2 SO  red --------> 5 RxD yel
    3 SI  org <-------- 4 TxD org
    4 SD  bwn --------> 2 CTS bwn
    5 SC  grn <-------- 6 RTS grn
    6 GND blu <-------> 1 GND blk

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
  often on /dev/ttyUSB0.
- start your UART program on your other computer. For Linux, screen works:
    `screen /dev/ttyUSB0 115200`
- type away!

## todo

- see how this setup holds under max load
- use oscilloscope to see what this looks like over the wire
- from GBA, echo proper return back, so a return doesn't go to the first column
  of the current line
- add simple cross-platform Python repl

## acknowledgement

- The UART-specific code was slightly adapted from Adrian O'Grady:
  https://web.archive.org/web/20050425075428/http://www.fivemouse.com/gba/
- The console-like text behaviour was ripped from pandaforth:
  https://github.com/iansharkey/pandaforth

## license

This repo is distributed under the MIT license. For licensing terms, see the
LICENSE file in the root of this repo or go to
http://opensource.org/licenses/MIT
