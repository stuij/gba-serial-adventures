#include <string.h>
#include <tonc.h>

#include "console.h"
#include "uart.h"

// registers for print
// SIO_CNT
struct reg_field siocnt_fields[] = {
  { 2, "baud rate" },
  { 1, "cts" },
  { 1, "parity"},
  { 1, "snd flag"},
  { 1, "rcv flag"},
  { 1, "error flag"},
  { 1, "data length"},
  { 1, "fifo enable"},
  { 1, "par enable"},
  { 1, "snd enable"},
  { 1, "rcv enable"},
  { 1, "must be 1"},
  { 1, "must be 1"},
  { 1, "irq enable"},
  { 1, "not used, 0"},
};

struct reg siocnt = { "SIOCNT", 16, siocnt_fields };

// RCNT
// Not clear what the order of the serial signals is.
// When idle, what is labeled as SI,SO (order taken from GBATEK) is low, so that
// would suggest one of them is SD. Should be easy enough to figure out.
struct reg_field rcnt_fields[] = {
  { 1, "SC (?)"},
  { 1, "SD (?)"},
  { 1, "SI (?)"},
  { 1, "SO (?)"},
  { 5, "not used rw"},
  { 5, "not used ro"},
  { 1, "not used rw"},
  { 1, "0 in uart"}
};

struct reg rcnt = { "RCNT", 16, rcnt_fields };

// uart IRQ routine
void handle_uart_ret() {

  // the error bit is reset when reading REG_SIOCNT
  if (REG_SIOCNT & SIO_ERROR) {
    write_line("SIO error\n");
  }

  // receiving data is time-sensitive so we handle this first without wasting CPU
  // cycles on say writing to the console
  if (!(REG_SIOCNT & SIO_RECV_DATA)) {
    // reserve an arbitrary amount of bytes for the rcv buffer
    char in[4096];
    u32 size = rcv_uart_ret(in);

    // null-terminating so we can write to the console with write_line
    in[size] = 0;
    write_line(in);

    // send line back over serial line
    snd_uart_ret(in, size);
  }

  if (!(REG_SIOCNT & SIO_SEND_DATA)) {
    // do something
  }
}

// uart IRQ routine
void handle_uart_gbaser() {
  // the error bit is reset when reading REG_SIOCNT
  if (REG_SIOCNT & SIO_ERROR) {
    write_line("SIO error\n");
  }

  // receiving data is time-sensitive so we handle this first without wasting CPU
  // cycles on say writing to the console
  if (!(REG_SIOCNT & SIO_RECV_DATA)) {
    // reserve an arbitrary amount of bytes for the rcv buffer
    char in[4096];
    char gbaser_type = GBASER_ERROR;
    char gbaser_status = GBASER_ERROR;
    s32 len = rcv_uart_gbaser(in, &gbaser_type, &gbaser_status);

    // error processing received message, CRC mismatch
    if(len == -1) {
      snd_uart_gbaser("", 0, GBASER_CRC_ERROR);
    }

    // null-terminating so we can write to the console with write_line
    in[len] = 0;
    write_line(in);

    char ok[] = "'s all ok, mate";
    // send ack = 0 back over serial line
    snd_uart_gbaser(ok, strlen(ok), GBASER_OK);
  }

  if (!(REG_SIOCNT & SIO_SEND_DATA)) {
    // do something
  }
}

// I think the FIFO is completely transparent to the user code, compared to no
// FIFO. It basically buys you 3 extra sent chars worth of cycles.
void toggle_fifo() {
  REG_SIOCNT & SIO_USE_FIFO ?
      write_line("disabling fifo\n") :
      write_line("enabling fifo\n");

  // disabling the fifo will reset it
  REG_SIOCNT ^= SIO_USE_FIFO;
}

void help() {
  write_line("\nSTART: display help text\n");
  write_line("SELECT: toggle fifo\n");
  write_line("LEFT: set ret IRQ handler\n");
  write_line("RIGHT: set len IRQ handler\n");
  write_line("A to write to screen\n");
  write_line("B to send over uart\n");
  write_line("L to print SIOCNT\n");
  write_line("R to print RCNT\n\n");
}

s32 main() {
  // Set to UART mode
  init_uart(SIO_BAUD_115200);

	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;
  
	// Base TTE init for tilemaps
	tte_init_se(
		0,						        // Background number (0-3)
		BG_CBB(0)|BG_SBB(31),	// BG control
		0,					        	// Tile offset (special cattr)
		CLR_WHITE,		        // Ink color
		14,						        // BitUnpack offset (on-pixel = 15)
		NULL,			        		// Default font (sys8) 
		NULL);					      // Default renderer (se_drawg_s)

  write_line(".. and now we wait ..\n\n\n\n");
  write_line("ready to receive from uart\n\n");
  help();

	irq_init(NULL);
  irq_add(II_SERIAL, handle_uart_ret);
	irq_add(II_VBLANK, NULL);

	while(1)
	{
		VBlankIntrWait();
		key_poll();

    if(key_hit(KEY_A)) {
      write_line("That tickles!\n");
    }
    if(key_hit(KEY_B)) {
      snd_uart_ret("some data\n", 10);
    }
    if(key_hit(KEY_LEFT)) {
      write_line("setting ret uart irq handler\n");
      irq_add(II_SERIAL, handle_uart_ret);
    }
    if(key_hit(KEY_RIGHT)) {
      write_line("setting len uart irq handler\n");
      irq_add(II_SERIAL, handle_uart_gbaser);
    }
    if(key_hit(KEY_SELECT)) {
      toggle_fifo();
    }
    if(key_hit(KEY_START)) {
      help();
    }
    if(key_hit(KEY_L)) {
      print_register(&siocnt, REG_SIOCNT);
    }
    if(key_hit(KEY_R)) {
      print_register(&rcnt, REG_RCNT);
    }
  }
}
