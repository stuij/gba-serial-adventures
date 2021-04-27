#ifndef UART_H
#define UART_H

#include <tonc.h>

void init_uart(u16 UART);
s32 rcv_uart_ret(char in[]);
void snd_uart_ret(char out[], s32 len);
s32 rcv_uart_gbaser(struct circ_buff* circ, char* type, char* status);
void snd_uart_gbaser(char out[], s32 len, char type);

void snd_char(s32 character);
s32 rcv_char(void);

// rcv buffer things
#define UART_RCV_BUFFER_SIZE 256
extern char g_rcv_buffer[UART_RCV_BUFFER_SIZE];
extern struct circ_buff g_uart_rcv_buffer;

// irq handlers
void handle_uart_ret();
void handle_uart_gbaser();

#define dputchar snd_char

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
#define GBASER_UNDEFINED '\x00'
#define GBASER_STRING '\x01'
#define GBASER_BINARY '\x02'
#define GBASER_MULTIBOOT '\x03'
#define GBASER_OK '\xFF'
#define GBASER_ERROR '\xFE'
#define GBASER_CRC_ERROR '\xFD'

#endif // UART_H
