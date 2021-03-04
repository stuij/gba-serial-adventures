#ifndef CONSOLE_H
#define CONSOLE_H

#include <tonc.h>

// print registers
struct reg_field {
  int size;
  char* name;
};

struct reg {
  char* name;
  int size;
  struct reg_field* fields;
};

void write_console_char(u32 ch);
void write_console_line(const char*);
void write_console_line_circ(struct circ_buff* buff);
void print_register(struct reg* reg, u32 value);
void printc (char* format, ...);

#endif // CONSOLE_H
