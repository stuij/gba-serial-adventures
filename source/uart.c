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


// rcv_uart expects the first byte to be the size of the transfer
unsigned int rcv_uart(unsigned char in[])
{
  // sd is assumed to be low
  // wait until we have a full byte (the recv data flag will go to 0 and sd will
  // go high)
  while(REG_SIOCNT & 0x0020);
  int size = (int)REG_SIODATA8;

  for(int i = 0; i < size; i++) {
    while(REG_SIOCNT & 0x0020);

    // Return the character in the data register
    in[i] = (unsigned char)REG_SIODATA8;
  }

  return size;
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
