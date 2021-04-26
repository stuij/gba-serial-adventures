#include <stdarg.h>
#include <stdio.h>
#include <tonc.h>

#include "circular_buffer.h"
#include "console.h"

// Console Data
u16 console[20][30];

// Cursor position
s32 row,col;

// fn lifted from pandaforth
void write_char(u32 ch) {
	s32 lastrow = row;
	s32 x,y,i;
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

void write_console_line(const char* line) {
  while (*line)
    write_char((u32)(*line++));
}

void write_console_line_circ(struct circ_buff* buff) {
  char out;
  while (read_circ_char(buff, &out))
    write_char((u32)out);
}

// print to console
void printc (char* format, ...) {
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, sizeof(buffer), format, args);
  va_end (args);
  write_console_line(buffer);
}


// print registers
void print_register(struct reg* reg, u32 value) {
  s32 start_bit = 0;
  s32 field_size = 0;
  s32 size = reg->size;
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
