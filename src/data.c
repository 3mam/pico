#include "data.h"

const ui16 palette0[] = {0, 0xF800, 0xFFFE, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0,      0,      0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0,      0,      0, 0, 0, 0, 0, 0};
const ui16* palette[] = {palette0, palette0, palette0, palette0,
                         palette0, palette0, palette0, palette0};

const ui8 sprite[] = {
#include "data.txt"
};

const ui16 sprite_width = 16;

const struct cell cell[] = {
    {0, 0, 8, 8},
    {8, 0, 16, 16},
    {0, 0, 16, 8},
};

struct data data = {.palette = palette,
                    .sprite = sprite,
                    .sprite_width = sprite_width,
                    .cell = cell};