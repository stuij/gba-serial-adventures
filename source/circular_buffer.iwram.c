#include "circular_buffer.h"

void init_circ_buff(struct circ_buff* circ, char* buff, int max) {
  circ->cur_size = 0;
  circ->max_size = max;
  circ->read_index = 0;
  circ->write_index = 0;
  circ->buffer = buff;
}

bool write_circ_char(struct circ_buff* circ, char data) {
  int write_index = circ->write_index;
  if(circ->cur_size < circ->max_size) {
    // for concurrency reasons, first write byte, then update occupied size
    circ->buffer[write_index] = data;
    circ->cur_size++;

    circ->write_index = write_index == circ->max_size - 1 ? 0 : ++write_index;
    return true;
  }
  return false;
}

bool read_circ_char(struct circ_buff* circ, char* data) {
  int read_index = circ->read_index;
  if(circ->cur_size > 0) {
    // first read byte, then decrease occupied size
    *data = circ->buffer[read_index];
    circ->cur_size--;

    circ->read_index = read_index == circ->max_size - 1 ? 0 : ++read_index;
    return true;
  }
  return false;
}

s32 circ_bytes_available(struct circ_buff* circ) {
  return circ->cur_size;
}
