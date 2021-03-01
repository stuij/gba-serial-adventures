#include "uart.h"
#include "console.h"

void init_uart(unsigned short uart) {
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
int rcv_uart_ret(unsigned char in[]) {
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

unsigned int crc32(unsigned int crc, unsigned char *buf, size_t len) {
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (int i = 0; i < 8; i++)
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}

// rcv_uart_len expects the first byte to be the size of the transfer
int rcv_uart_len(unsigned char in[]) {
  int len = 0;
  unsigned char type;
  unsigned char data;
  unsigned int our_crc = 0;
  unsigned int their_crc = 0;

  for(int i = 0; i < 4; i++) {
    // wait until we have a full byte (the recv data flag will go to 0 and sd will
    // go high)
    while(REG_SIOCNT & 0x0020);
    len = (len << 8) | REG_SIODATA8;
  }

  while(REG_SIOCNT & 0x0020);
  type = REG_SIODATA8;

  for(int i = 0; i < len; i++) {
    while(REG_SIOCNT & 0x0020);
    data = (unsigned char)REG_SIODATA8;
    // Return the character in the data register
    in[i] = data;
  }

  for(int i = 0; i < 4; i++) {
    while(REG_SIOCNT & 0x0020);
    their_crc = (their_crc << 8) | REG_SIODATA8;
  }

  // check crc
  our_crc = crc32(0, in, len);
  if(their_crc != our_crc) {
    printc("ERROR: CRC mismatch!\n");
    printc("message length: 0x%08x\n", len);
    printc("       our CRC: 0x%08x\n", crc32(our_crc, in, len));
    printc("     their CRC: 0x%08x\n", their_crc);
    return -1;
}

  return len;
}

void snd_uart(unsigned char out[], unsigned int size) {
  for(int i = 0; i < size; i++) {
    // Wait until the send queue is empty
    while(REG_SIOCNT & SIO_SEND_DATA);

    // Bung our byte into the data register
    REG_SIODATA8 = out[i];
  }
}
