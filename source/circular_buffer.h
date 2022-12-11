#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <tonc.h>

// a simple circular buffer implementation
// I think it should be interrupt safe for single producer, single consumer
struct circ_buff {
  int   cur_size;
  int   max_size;
  int   read_index;
  int   write_index;
  char* buffer;
};

void init_circ_buff(struct circ_buff* circ, char* buff, int max);
bool write_circ_char(struct circ_buff* circ, char data);
bool read_circ_char(struct circ_buff* circ, char* data);
s32 circ_bytes_available(struct circ_buff* circ);

#endif // CIRCULAR_BUFFER_H
