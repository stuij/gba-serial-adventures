#include <tonc.h>

#include "console.h"
#include "uart.h"

// uart IRQ routine
void handle_uart() {

  // the error bit is reset when reading REG_SIOCNT
  if (REG_SIOCNT & SIO_ERROR) {
    write_line("SIO error\n");
  }

  // receiving data is time-sensitive so we handle this first without wasting CPU
  // cycles on say writing to the console
  if (!(REG_SIOCNT & SIO_RECV_DATA)) {
    // reserve an arbitrary amount of bytes for the rcv buffer
    unsigned char in[4096];
    unsigned int size = rcv_uart(in);

    // null-terminating so we can write to the console with write_line
    in[size] = 0;
    write_line((char*) in);

    // send line back over serial line
    snd_uart(in, size);
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

int main() {
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

  write_line(".. and now we wait ..\n\n\n\n\n");
  write_line("ready to receive from uart\n\n\n");
  write_line("press:\n\n");
  write_line("- A to write to screen\n");
  write_line("- B to send over uart\n");
  write_line("- SELECT to toggle fifo\n\n");

	irq_init(NULL);
  irq_add(II_SERIAL, handle_uart);
	irq_add(II_VBLANK, NULL);

	while(1)
	{
		VBlankIntrWait();
		key_poll();

    if(key_hit(KEY_A)) {
      write_line("That tickles!\n");
    }

    if(key_hit(KEY_B)) {
      snd_uart((unsigned char*)"some data\n", 10);
    }

    if(key_hit(KEY_SELECT)) {
      toggle_fifo();
    }
  }
}
