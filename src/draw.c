#include "draw.h"
#include <math.h>
#include <string.h>
#include "data.h"

static struct {
  struct {
    i16 x;
    i16 y;
    i16 width;
    i16 height;
  } screen;

  struct {
    i16 x;
    i16 y;
    i16 width;
    i16 height;
  } clip;

  ui8* canvas;
  ui16 canvas_size;

  ui8* palette;
  ui8 palette_index : 3;

  ui16* video_buffer;

  ui8 z : 3;
  ui8 color : 5;
  ui8 flip : 3;

} self;

void init(i16 width, i16 height, void* video_buffer) {
  self.screen.x = 0;
  self.screen.y = 0;
  self.screen.width = width;
  self.screen.height = height;
  self.clip.x = -1;
  self.clip.y = -1;
  self.clip.width = width;
  self.clip.height = height;
  self.canvas_size = (ui16)(width * height);
  self.canvas = MAKE(ui8, self.canvas_size);
  self.palette = MAKE(ui8, 31);
  self.video_buffer = (ui16*)video_buffer;
  self.z = 3;
  self.color = 1;
  self.flip = 0;
  self.palette_index = 0;

  draw.palr();
}

void update(void) {
  for (ui16 i = self.canvas_size; i--;) {
    self.video_buffer[i] =
        data.palette[self.palette_index][(self.canvas[i] & 0b00011111)];
  }
}

void color(ui8 c) {
  self.color = c;
}

void pset(i16 x, i16 y, ui8 c) {
  x -= self.screen.x;
  y -= self.screen.y;
  if (x < self.screen.width && x > -1 && y < self.screen.height && y > -1 &&
      x < self.clip.width && x > self.clip.x && y < self.clip.height &&
      y > self.clip.y) {
    i16 xy = ((y * self.screen.width) + x);
    if (((self.canvas[xy] & 0b11100000) >> 5) > self.z) {
      return;
    }
    self.canvas[xy] = (self.z << 5) | self.palette[c];
  }
}

ui8 pget(i16 x, i16 y) {
  if (x < self.screen.width && x > -1 && y < self.screen.height && y > -1) {
    return (ui8)(self.canvas[(y * self.screen.width) + x] & 0b00011111);
  }
  return 0;
}

void palr(void) {
  for (ui8 i = 31; i--;) {
    self.palette[i] = i;
    // self.screen.palette[i] = (ui8)(data.palette[i]) << 16 |
    //                       (ui8)(data.palette[i] >> 8) << 8 |
    //                     (ui8)(data.palette[i] >> 16);
  }
  self.palette[0] = 32;
  self.palette_index = 0;
}

void pal(ui8 p) {
  self.palette_index = p;
}

void palt(ui8 c, bool t) {
  if (t) {
    self.palette[c] = c | 1 << 5;
  } else {
    self.palette[c] = c & 0b00011111;
  }
}

void zbuff(ui8 z) {
  self.z = z;
}

void cls(ui8 col) {
  for (ui16 n = self.canvas_size; n--;) {
    self.canvas[n] = 0b01100000 | col;
  }
}

void clip(i16 x, i16 y, i16 w, i16 h) {
  self.clip.x = x - 1;
  self.clip.y = y - 1;
  self.clip.width = x + w;
  self.clip.height = y + h;
}

void clipr() {
  self.clip.x = -1;
  self.clip.y = -1;
  self.clip.width = self.screen.width;
  self.clip.height = self.screen.height;
}

void camera(i16 x, i16 y) {
  self.screen.x = x;
  self.screen.y = y;
}

void spr(ui16 n, i16 x, i16 y) {
  ui8 c = 0;
  ui16 sw = data.cell[n].end_y;
  ui16 sh = data.cell[n].end_x;
  for (ui16 sy = data.cell[n].start_y; sy < sw; sy++) {
    for (ui16 sx = data.cell[n].start_x; sx < sh; sx++) {
      switch (self.flip) {
        case 3:
          c = self.palette[data.sprite[(-sy + sw - 1) * data.sprite_width +
                                       sx]];
          break;
        case 2:
          c = self.palette[data.sprite[(-sy + sw - 1) * data.sprite_width +
                                       (-sx + sh - 1)]];
          break;
        case 1:
          c = self.palette[data.sprite[sy * data.sprite_width +
                                       (-sx + sh - 1)]];
          break;
        default:
          c = self.palette[data.sprite[sy * data.sprite_width + sx]];
          break;
      }
      if ((c & 0b00100000) == 32) {
        continue;
      }
      draw.pset(x + sx, y + sy, c);
    }
  }
}

void flip(ui8 f) {
  self.flip = f;
}

void line(i16 x0, i16 y0, i16 x1, i16 y1) {
  i16 dx = abs(x1 - x0);
  i16 dy = -abs(y1 - y0);
  i16 sx = x0 < x1 ? 1 : -1;
  i16 sy = y0 < y1 ? 1 : -1;
  i16 err = dx + dy;
  i16 e2 = 0;

  for (;;) {
    draw.pset(x0, y0, self.color);

    if (x0 == x1 && y0 == y1) {
      break;
    }

    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void rect(i16 x0, i16 y0, i16 x1, i16 y1) {
  draw.line(x0, y0, x1, y0);
  draw.line(x1, y0, x1, y1);
  draw.line(x1, y1, x0, y1);
  draw.line(x0, y1, x0, y0);
}

void rectf(i16 x0, i16 y0, i16 x1, i16 y1) {
  for (i16 y = y0; y < y1 + 1; y++) {
    for (i16 x = x0; x < x1 + 1; x++) {
      draw.pset(x, y, self.color);
    }
  }
}

void circ(i16 x0, i16 y0, i16 r) {
  i16 x = -r;
  i16 y = 0;
  i16 err = 2 - 2 * r;  // r * 2 - 2

  for (; x < -1;) {
    r = err;
    if (r > x) {
      x++;
      err += x * 2 + 1;
    }
    if (r <= y) {
      y++;
      err += y * 2 + 1;
    }

    draw.pset(x0 + x, y0 - y, self.color);
    draw.pset(x0 + x, y0 + y - 1, self.color);
    draw.pset(x0 - x - 1, y0 - y, self.color);
    draw.pset(x0 - x - 1, y0 + y - 1, self.color);
  }
}

void circf(i16 x0, i16 y0, i16 r) {
  i16 x = -r;
  i16 y = 0;
  i16 err = 2 - 2 * r;  // r * 2 - 2
  i16 y_repeat = 0;

  for (; x < 0;) {
    r = err;
    if (r > x) {
      x++;
      err += x * 2 + 1;
    }
    if (r <= y) {
      y++;
      err += y * 2 + 1;
    }

    if (y_repeat == y) {
      continue;
    }
    y_repeat = y;

    for (i16 i = x; i < 0; i++) {
      draw.pset(x0 + x - i - 1, y0 - y, self.color);
      draw.pset(x0 + x - i - 1, y0 + y - 1, self.color);
      draw.pset(x0 - x + i, y0 - y, self.color);
      draw.pset(x0 - x + i, y0 + y - 1, self.color);
    }
  }
}

void tri(i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2) {
  draw.line(x0, y0, x1, y1);
  draw.line(x1, y1, x2, y2);
  draw.line(x0, y0, x2, y2);
}

static void sort2dvectors(f32 list[3][2]) {
  for (i16 i = 0; i < 3; i++) {
    for (i16 j = 0; j < 3; j++) {
      if (i != j) {
        f32 x1 = list[i][0];
        f32 y1 = list[i][1];
        f32 x2 = list[j][0];
        f32 y2 = list[j][1];
        if (y2 > y1) {
          f32 tmp[2];
          tmp[0] = list[i][0];
          tmp[1] = list[i][1];
          list[i][0] = list[j][0];
          list[i][1] = list[j][1];
          list[j][0] = tmp[0];
          list[j][1] = tmp[1];
        } else if (y2 == y1) {
          if (x2 > x1) {
            f32 tmp[2];
            tmp[0] = list[i][0];
            tmp[1] = list[i][1];
            list[i][0] = list[j][0];
            list[i][1] = list[j][1];
            list[j][0] = tmp[0];
            list[j][1] = tmp[1];
          }
        }
      }
    }
  }
}

void trif(i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2) {
  f32 list[3][2];
  list[0][0] = (f32)(x0);
  list[0][1] = (f32)(y0);
  list[1][0] = (f32)(x1);
  list[1][1] = (f32)(y1);
  list[2][0] = (f32)(x2);
  list[2][1] = (f32)(y2);

  sort2dvectors(list);

  f32 xs = list[0][0];
  f32 xe = list[0][0];
  f32 vx1 = (list[1][0] - list[0][0]) / (list[1][1] - list[0][1]);
  f32 vx2 = (list[2][0] - list[1][0]) / (list[2][1] - list[1][1]);
  f32 vx3 = (list[2][0] - list[0][0]) / (list[2][1] - list[0][1]);

  if (floor(list[1][1] - list[0][1]) == 0) {
    vx2 = vx3;
    xe = list[1][0];
    vx3 = (list[2][0] - list[1][0]) / (list[2][1] - list[1][1]);
  }

  for (f32 y = list[0][1]; y < list[2][1]; y++) {
    f32 l = sqrt((xs - xe) * (xs - xe));

    for (f32 x = 0; x < -floor(-(l)); x++) {
      draw.pset((i16)(x + floor(fmin(xs, xe))), (i16)(y), self.color);
    }

    if (y < list[1][1]) {
      xs += vx1;
    } else if (y >= list[1][1]) {
      xs += vx2;
    }
    xe += vx3;
  }
}

struct draw draw = {.init = init,
                    .update = update,
                    .pset = pset,
                    .pget = pget,
                    .pal = pal,
                    .palr = palr,
                    .palt = palt,
                    .zbuff = zbuff,
                    .cls = cls,
                    .color = color,
                    .spr = spr,
                    .flip = flip,
                    .clip = clip,
                    .clipr = clipr,
                    .camera = camera,
                    .line = line,
                    .rect = rect,
                    .rectf = rectf,
                    .circ = circ,
                    .circf = circf,
                    .tri = tri,
                    .trif = trif};
