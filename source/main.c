#include <tonc.h>

#include "console.h"
#include "uart.h"

int main() {
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

  // Set to UART mode
  init_uart(SIO_BAUD_115200);
  
	// Base TTE init for tilemaps
	tte_init_se(
		0,						        // Background number (0-3)
		BG_CBB(0)|BG_SBB(31),	// BG control
		0,					        	// Tile offset (special cattr)
		CLR_WHITE,		        // Ink color
		14,						        // BitUnpack offset (on-pixel = 15)
		NULL,			        		// Default font (sys8) 
		NULL);					      // Default renderer (se_drawg_s)

  unsigned char in;
  
	while(1)
	{
		// VBlankIntrWait();
		// key_poll();

    // if(key_hit(KEY_LEFT)) {
    //  write_char('a');
    // }

    // if(key_hit(KEY_RIGHT))
    //   write_char('\n');    

    in = rcv_char();
    write_char((int)in);
    snd_char(in);
  }
}
