#include <string.h>
#include <tonc.h>

#include "circular_buffer.h"
#include "console.h"
#include "uart.h"

// I think the FIFO is completely transparent to the user code, compared to no
// FIFO. It basically buys you 3 extra sent chars worth of cycles.
void toggle_fifo() {
  REG_SIOCNT & SIO_USE_FIFO ?
      write_console_line("disabling fifo\n") :
      write_console_line("enabling fifo\n");

  // disabling the fifo will reset it
  REG_SIOCNT ^= SIO_USE_FIFO;
}

void help() {
  write_console_line("\nSTART: display help text\n");
  write_console_line("SELECT: toggle fifo\n");
  write_console_line("LEFT: set passthrough IRQ mode\n");
  write_console_line("RIGHT: set Gbaser IRQ mode\n");
  write_console_line("A to write to screen\n");
  write_console_line("B to send over uart\n");
  write_console_line("L to print SIOCNT\n");
  write_console_line("R to print RCNT\n\n");
}

void do_keys() {
  key_poll();

    if(key_hit(KEY_A)) {
      write_console_line("That tickles!\n");
    }
    if(key_hit(KEY_B)) {
      snd_uart_ret("some data\n", 10);
    }
    if(key_hit(KEY_LEFT)) {
      write_console_line("passthrough uart irq mode set\n");
      irq_add(II_SERIAL, handle_uart_ret);
    }
    if(key_hit(KEY_RIGHT)) {
      write_console_line("Gbaser uart irq mode set\n");
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

s32 main() {
  // set up display
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

  // Set to UART mode
  init_circ_buff(&g_uart_rcv_buffer, g_rcv_buffer, UART_RCV_BUFFER_SIZE);
  init_uart(SIO_BAUD_115200);

  // set irqs
	irq_init(NULL);
  irq_add(II_SERIAL, handle_uart_gbaser);
	irq_add(II_VBLANK, NULL);

  // welcome text
  write_console_line(".. and now we wait ..\n\n\n");
  write_console_line("ready to receive from uart\n");
  write_console_line("uart mode: Gbaser\n");
  help();

  // main loop
	while(1)
	{
		VBlankIntrWait();
    do_keys();

    if(circ_bytes_available(&g_uart_rcv_buffer)) {
      write_console_line_circ(&g_uart_rcv_buffer);
    }
  }
}
