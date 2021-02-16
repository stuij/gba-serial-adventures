#include "uart.h"

void init_uart(unsigned short uart)
{
  // Stick a character in the data register. This stops the GBA transmitting a
  // Character as soon as it goes into UART mode (?!?)
  REG_SIODATA8 = 'A';

  // Now to go into UART mode
  REG_RCNT = 0;
  REG_SIOCNT = 0;

  // for if you do or don't have the CTS/RTS wires
  // see README.md
#if FLOW_CONTROL
  REG_SIOCNT = uart | SIO_CTS | SIO_LENGTH_8 | SIO_SEND_ENABLE
             | SIO_RECV_ENABLE | SIO_USE_UART;
#else
  REG_SIOCNT = uart | SIO_LENGTH_8 | SIO_SEND_ENABLE
             | SIO_RECV_ENABLE | SIO_USE_UART;
#endif

  // With any luck, we should now be able to talk to a PC.
}


// we collect bytes in `in` until we see a `return`
unsigned int rcv_uart(unsigned char in[])
{
  // make sure there's no '\n' in `last`
  unsigned char last = ' ';
  int i;
  for(i = 0; i < 4096 && last != '\n'; i++) {
    // sd is assumed to be low
    // wait until we have a full byte (the recv data flag will go to 0 and sd will
    // go high)
    while(REG_SIOCNT & 0x0020);

    // save the char from the sio data register
    last = (unsigned char)REG_SIODATA8;
    in[i] = last;
  }

  return i;
}


void snd_uart(unsigned char out[], unsigned int size)
{
  for(int i = 0; i < size; i++) {
    // Wait until the send queue is empty
    while(REG_SIOCNT & 0x0010);

    // Bung our byte into the data register
    REG_SIODATA8 = out[i];
  }
}
