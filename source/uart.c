#include "uart.h"
#include "console.h"

void init_uart(u16 uart) {
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

/// 'check for return' versions of rcv_uart/snd_uart

// we collect bytes in `in` until we see a `return`
s32 rcv_uart_ret(char in[]) {
  // make sure there's no '\n' in `last`
  char last = ' ';
  s32 i;
  for(i = 0; i < 4096 && last != '\n'; i++) {
    // sd is assumed to be low
    // wait until we have a full byte (the recv data flag will go to 0 and sd will
    // go high)
    while(REG_SIOCNT & SIO_RECV_DATA);

    // save the char from the sio data register
    last = (char)REG_SIODATA8;
    in[i] = last;
  }
  return i;
}

void snd_uart_ret(char out[], s32 len) {
  for(s32 i = 0; i < len; i++) {
    // Wait until the send queue is empty
    while(REG_SIOCNT & SIO_SEND_DATA);

    // Bung our byte into the data register
    REG_SIODATA8 = out[i];
  }
}


/// Gbaser protocol versions of rcv_uart/snd_uart
// see the readme for the protocol spec

// this is the zlib crc32 variant
u32 crc32(u32 crc, char *buf, size_t len) {
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (s32 i = 0; i < 8; i++)
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}

s32 rcv_uart_gbaser(char in[], char* type, char* status) {
  u32 len = 0;
  char data;
  u32 our_crc = 0;
  u32 their_crc = 0;
  *status = GBASER_ERROR;

  // first get message type - 1 byte
  while(REG_SIOCNT & 0x0020);
  *type = REG_SIODATA8;

  // then get data length - 4 bytes
  for(s32 i = 0; i < 4; i++) {
    // wait until we have a full byte (the recv data flag will go to 0 and sd will
    // go high)
    while(REG_SIOCNT & 0x0020);
    len = (len >> 8) | (REG_SIODATA8 << 24);
  }

  // get data - len bytes
  for(s32 i = 0; i < len; i++) {
    while(REG_SIOCNT & 0x0020);
    data = REG_SIODATA8;
    // Return the character in the data register
    in[i] = data;
  }

  // get crc - 4 bytes
  for(s32 i = 0; i < 4; i++) {
    while(REG_SIOCNT & 0x0020);
    their_crc = (their_crc >> 8) | (REG_SIODATA8 << 24);
  }

  // check crc
  our_crc = crc32(0, in, len);
  if(their_crc != our_crc) {

    printc("message length: 0x%08x\n", len);
    printc("       our CRC: 0x%08x\n", crc32(our_crc, in, len));
    printc("     their CRC: 0x%08x\n", their_crc);
    *status = GBASER_CRC_ERROR;
    return -1;
  } else {
    *status = GBASER_OK;
  }

  return len;
}

void snd_uart_gbaser(char out[], s32 len, char type) {
  // calculate CRC
  u32 crc = crc32(0, out, len);

  // first send message type - 1 byte
  // wait until the send queue is empty
  while(REG_SIOCNT & SIO_SEND_DATA);
  // bung our byte into the data register
  REG_SIODATA8 = type;

  // send data length - 4 bytes
  for(s32 i = 0; i < 4; i++) {
    while(REG_SIOCNT & SIO_SEND_DATA);
    REG_SIODATA8 = ((char*)&len)[i];
  }

  // send data
  for(s32 i = 0; i < len; i++) {
    while(REG_SIOCNT & SIO_SEND_DATA);
    REG_SIODATA8 = out[i];
  }

  for(s32 i = 0; i < 4; i++) {
    while(REG_SIOCNT & SIO_SEND_DATA);
    REG_SIODATA8 = ((char*)&crc)[i];
  }
}
