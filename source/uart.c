#include "uart.h"

void init_uart(unsigned short uart)
{
  // clear out SIO control registers
  REG_RCNT = 0;
  REG_SIOCNT = 0;

  // If you want to enable fifo straight away, enable UART with fifo
  // disabled as it is reset/initialized when off. On another line in the source
  // file, set SIO_USE_FIFO to 1.

  // for if you do or don't have the CTS/RTS wires; see README.md
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
unsigned int rcv_uart_ret(unsigned char in[])
{
  // make sure there's no '\n' in `last`
  unsigned char last = ' ';
  int i;
  for(i = 0; i < 4096 && last != '\n'; i++) {
    // sd is assumed to be low
    // wait until we have a full byte (the recv data flag will go to 0 and sd will
    // go high)
    while(REG_SIOCNT & SIO_RECV_DATA);

    // save the char from the sio data register
    last = (unsigned char)REG_SIODATA8;
    in[i] = last;
  }
  return i;
}

// rcv_uart_len expects the first byte to be the size of the transfer
unsigned int rcv_uart_len(unsigned char in[]) {
  // wait until we have a full byte (the recv data flag will go to 0 and sd will
  // go high)
  while(REG_SIOCNT & 0x0020);
  int size = (int)REG_SIODATA8;

  for(int i = 0; i < size; i++) {
    // Wait until we have a full byte (The recv data flag will go to 0)
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
    while(REG_SIOCNT & SIO_SEND_DATA);

    // Bung our byte into the data register
    REG_SIODATA8 = out[i];
  }
}
