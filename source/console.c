#include <tonc.h>
#include "stdio.h"

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

void write_line(const char* line) {
  while (*line) write_char((int)(*line++));
}

  // TODO: this is practically an implementation of a console printf
  // char str[20];
  // sprintf(str, "%x\n", REG_SIOCNT);
  // write_line(str);

