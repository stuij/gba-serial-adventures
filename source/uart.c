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

int rcv_char(void)
{
  // Set the receive data flag to empty
  REG_SIOCNT = REG_SIOCNT | 0x0020;

  // We're using CTS so we must send a LO on the SD terminal to show we're ready
  REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);

  // Wait until we have a full byte (The recv data flag will go to 0)
  while(REG_SIOCNT & 0x0020);

  // Return the character in the data register
  return (int)REG_SIODATA8;
}

void snd_char(int character)
{
  // Wait until we have a CTS signal
  while(REG_RCNT & 0x0010);

  // Bung our byte into the data register
  REG_SIODATA8 = (unsigned char)character;
}
