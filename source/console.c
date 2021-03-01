#include <stdarg.h>
#include <stdio.h>
#include <tonc.h>

#include "console.h"

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

// print to console
void printc (char * format, ...) {
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, sizeof(buffer), format, args);
  write_line(buffer);
  va_end (args);
}


// print registers
void print_register(struct reg* reg, int value) {
  int start_bit = 0;
  int field_size = 0;
  int size = reg->size;
  char bit_string[6];

  printc("\nreg %s: 0x%04x\n", reg->name, value);
  for(int field_no = 0; start_bit < size; field_no++) {
    struct reg_field field = reg->fields[field_no];
    field_size = field.size;

    if (field_size == 1)
      sprintf(bit_string, "%02d   ", start_bit);
    else
      sprintf(bit_string, "%02d-%02d", start_bit, start_bit + field_size - 1);

    printc("%s %-12s: %2x\n",
           bit_string,
           field.name,
           (value >> start_bit) & ((1 << (field_size)) - 1));

    start_bit += field_size;
  }
  printc("\n");
};
