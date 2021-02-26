#include <tonc.h>

#include "console.h"
#include "uart.h"

// uart IRQ routine
void handle_uart() {
  // TODO: dispatch on interrupt signal:
  // - incoming byte
  // - done sending
  // - SIO error handling
  if (!(REG_SIOCNT & 0x0020)) {
    // reserve an arbitrary amount of bytes for the rcv buffer
    unsigned char in[4096];
    unsigned int size = rcv_uart(in);

    // null-terminating so we can print to the console with write_line
    in[size] = 0;
    write_line((char*) in);

    // send line back over serial line
    snd_uart(in, size);
  }
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

  write_line(".. and now we wait ..\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	irq_init(NULL);
  irq_add(II_SERIAL, handle_uart);
	irq_add(II_VBLANK, NULL);

	while(1)
	{
		VBlankIntrWait();
		key_poll();

    if(key_hit(KEY_A)) {
      write_line("That tickles!\nStop pressing A please..\n");
    }
  }
}

