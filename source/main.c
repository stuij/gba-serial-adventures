#include <tonc.h>

// Console Data
u16 console[20][30];

// Cursor position
int row,col;

// fn lifted from pandaforth
void write_char(int ch) {
	int lastrow = row;
	int x,y,i;
	if (ch >= 32) { 
		console[row][col++] = ch;		
		if (col == 30) {
			col=0;
			row++;
			if (row == 20) row = 0;
			// Clean the next colon
			for(x=0; x<30; x++) console[row][x] = ' ';
		}
	} else if (ch == '\n') {
		col=0;
		row++;
		if (row == 20) row = 0;		
		// Clean the next colon
		for(x=0; x<30; x++) console[row][x] = ' ';
	} else if (ch == 0x08) {
		console[row][col--] = ' ';
		if (col < 0) {
			col = 29;
			row--;
			if (row < 0) row = 19;
		}
	}
	
	if (lastrow != row) {
		// Update the whole screen
		y = row+1;
		if (y > 19) y=0;
		for(i=0; i<20; i++) {
      tte_set_pos(0, i*8);
			for(x=0; x<30; x++) {
				tte_putc(console[y][x]);
			}
			y++;
			if (y > 19) y=0;
		}
	} else {
    tte_set_pos(0, 19*8);
		// Update only the current row
		for(x=0; x<30; x++) {
				tte_putc(console[row][x]);
		}
	}
}


int main() {
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);
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

	while(1)
	{
		VBlankIntrWait();
		key_poll();
    if(key_hit(KEY_LEFT)) {
      write_char('a');
    }

    if(key_hit(KEY_RIGHT))
      write_char('\n');    
  }
}
