#ifndef DATA_H
#define DATA_H

#include "macro.h"

struct cell {
  ui16 start_x;
  ui16 start_y;
  ui16 end_x;
  ui16 end_y;
};

struct data {
  const ui16** palette;
  const ui8* sprite;
  const ui16 sprite_width;
  const struct cell* cell;
};

extern struct data data;

#endif  // DATA_H
