#ifndef UART_H
#define UART_H

#include <tonc.h>

void init_uart(unsigned short UART);
int rcv_uart_ret(unsigned char in[]);
int rcv_uart_len(unsigned char in[]);
void snd_uart(unsigned char out[], unsigned int size);

#define dputchar snd_char

/* These are buggered right now
#define SIO_SET_SDHI      REG_RCNT &= 0x0020
#define SIO_SET_SDLO      REG_RCNT &= (0x0020 ^ 0xFFFF)
#define SIO_GET_SC        REG_RCNT &  0x0010
*/

// UART settings
#define SIO_USE_UART      0x3000

// Baud Rate
#define SIO_BAUD_9600     0x0000
#define SIO_BAUD_38400    0x0001
#define SIO_BAUD_57600    0x0002
#define SIO_BAUD_115200   0x0003

#define SIO_CTS           0x0004
#define SIO_PARITY_ODD    0x0008
#define SIO_SEND_DATA     0x0010
#define SIO_RECV_DATA     0x0020
#define SIO_ERROR         0x0040
#define SIO_LENGTH_8      0x0080
#define SIO_USE_FIFO      0x0100
#define SIO_USE_PARITY    0x0200
#define SIO_SEND_ENABLE   0x0400
#define SIO_RECV_ENABLE   0x0800
#define SIO_REQUEST_IRQ   0x4000

// GBuArt message
#define GBUART_STRING '\x00'
#define GBUART_RET_OK '\xFF'
#define GBUART_RET_ERROR '\xFE'
#define GBUART_RET_CRC_ERROR '\xFD'

#endif // UART_H
