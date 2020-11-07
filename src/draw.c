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
  self.z = 0;
  self.color = 1;
  self.flip = 0;
  self.palette_index = 0;

  draw.resetColorsTransparent();
}

void updateVideoBuffer(void) {
  for (ui16 i = self.canvas_size; i--;) {
    self.video_buffer[i] =
        data.palette[self.palette_index][(self.canvas[i] & 0b00011111)];
  }
}

void setColor(ui8 color) {
  self.color = color;
}

i8 encodeZBufferFromCanvas(ui8 c) {
  return (c >> 7 ? 0b11111100 : 0) | (c & 0b01100000) >> 5;
}

void setPixel(i16 x, i16 y, ui8 color) {
  x -= self.screen.x;
  y -= self.screen.y;
  if (x < self.screen.width && x > -1 && y < self.screen.height && y > -1 &&
      x < self.clip.width && x > self.clip.x && y < self.clip.height &&
      y > self.clip.y) {
    i16 xy = ((y * self.screen.width) + x);
    if (encodeZBufferFromCanvas(self.canvas[xy]) > self.z)
      return;
    self.canvas[xy] = (self.z << 5) | self.palette[color];
  }
}

ui8 getPixel(i16 x, i16 y) {
  if (x < self.screen.width && x > -1 && y < self.screen.height && y > -1) {
    return (ui8)(self.canvas[(y * self.screen.width) + x] & 0b00011111);
  }
  return 0;
}

void swapPallet(ui8 pallet_number) {
  self.palette_index = pallet_number;
}

void setColorToTransparent(ui8 color, bool transparent) {
  if (transparent)
    self.palette[color] = color | 1 << 5;
  else
    self.palette[color] = color & 0b00011111;
}

void resetColorsTransparent(void) {
  for (ui8 i = 31; i--;) {
    self.palette[i] = i;
    // self.screen.palette[i] = (ui8)(data.palette[i]) << 16 |
    //                       (ui8)(data.palette[i] >> 8) << 8 |
    //                     (ui8)(data.palette[i] >> 16);
  }
  self.palette[0] = 32;
  self.palette_index = 0;
}

void setZBuffer(i8 value) {
  self.z = value;
}

void clearScreen(ui8 color) {
  for (ui16 n = self.canvas_size; n--;) {
    self.canvas[n] = 0b10100000 | color;
  }
}

void clipping(reactangle dimensions) {
  self.clip.x = dimensions.x - 1;
  self.clip.y = dimensions.y - 1;
  self.clip.width = dimensions.x + dimensions.width;
  self.clip.height = dimensions.y + dimensions.height;
}

void resetClipping() {
  self.clip.x = -1;
  self.clip.y = -1;
  self.clip.width = self.screen.width;
  self.clip.height = self.screen.height;
}

void setCameraPosition(point position) {
  self.screen.x = position.x;
  self.screen.y = position.y;
}

void sprite(ui16 number, point position) {
  ui8 c = 0;
  ui16 sw = data.cell[number].end_y;
  ui16 sh = data.cell[number].end_x;
  for (ui16 sy = data.cell[number].start_y; sy < sw; sy++) {
    for (ui16 sx = data.cell[number].start_x; sx < sh; sx++) {
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
      draw.setPixel(position.x + sx, position.y + sy, c);
    }
  }
}

void flipSprite(ui8 position) {
  self.flip = position;
}

void line(point first, point second) {
  i16 dx = abs(second.x - first.x);
  i16 dy = -abs(second.y - first.y);
  i16 sx = first.x < second.x ? 1 : -1;
  i16 sy = first.y < second.y ? 1 : -1;
  i16 err = dx + dy;
  i16 e2 = 0;

  for (;;) {
    draw.setPixel(first.x, first.y, self.color);

    if (first.x == second.x && first.y == second.y) {
      break;
    }

    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      first.x += sx;
    }

    if (e2 <= dx) {
      err += dx;
      first.y += sy;
    }
  }
}

void rectangle(point first, point second) {
  draw.line(first, second);
  draw.line(first, second);
  draw.line(first, second);
  draw.line(first, second);
}

void filledRectangle(point first, point second) {
  for (i16 y = first.y; y < second.y + 1; y++) {
    for (i16 x = first.x; x < second.x + 1; x++) {
      draw.setPixel(x, y, self.color);
    }
  }
}

void circle(point position, i16 radius) {
  i16 x = -radius;
  i16 y = 0;
  i16 err = 2 - 2 * radius;  // r * 2 - 2

  for (; x < -1;) {
    radius = err;
    if (radius > x) {
      x++;
      err += x * 2 + 1;
    }
    if (radius <= y) {
      y++;
      err += y * 2 + 1;
    }

    draw.setPixel(position.x + x, position.y - y, self.color);
    draw.setPixel(position.x + x, position.y + y - 1, self.color);
    draw.setPixel(position.x - x - 1, position.y - y, self.color);
    draw.setPixel(position.x - x - 1, position.y + y - 1, self.color);
  }
}

void filledCircle(point position, i16 radius) {
  i16 x = -radius;
  i16 y = 0;
  i16 err = 2 - 2 * radius;  // r * 2 - 2
  i16 y_repeat = 0;

  for (; x < 0;) {
    radius = err;
    if (radius > x) {
      x++;
      err += x * 2 + 1;
    }
    if (radius <= y) {
      y++;
      err += y * 2 + 1;
    }

    if (y_repeat == y) {
      continue;
    }
    y_repeat = y;

    for (i16 i = x; i < 0; i++) {
      draw.setPixel(position.x + x - i - 1, position.y - y, self.color);
      draw.setPixel(position.x + x - i - 1, position.y + y - 1, self.color);
      draw.setPixel(position.x - x + i, position.y - y, self.color);
      draw.setPixel(position.x - x + i, position.y + y - 1, self.color);
    }
  }
}

void triangle(point first, point second, point third) {
  draw.line(first, second);
  draw.line(second, third);
  draw.line(first, third);
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

void filledTriangle(point first, point second, point third) {
  f32 list[3][2];
  list[0][0] = (f32)(first.x);
  list[0][1] = (f32)(first.y);
  list[1][0] = (f32)(second.x);
  list[1][1] = (f32)(second.y);
  list[2][0] = (f32)(third.x);
  list[2][1] = (f32)(third.y);

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
      draw.setPixel((i16)(x + floor(fmin(xs, xe))), (i16)(y), self.color);
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
                    .updateVideoBuffer = updateVideoBuffer,
                    .setPixel = setPixel,
                    .getPixel = getPixel,
                    .swapPallet = swapPallet,
                    .setColorToTransparent = setColorToTransparent,
                    .resetColorsTransparent = resetColorsTransparent,
                    .clearScreen = clearScreen,
                    .clipping = clipping,
                    .resetClipping = resetClipping,
                    .setColor = setColor,
                    .sprite = sprite,
                    .flipSprite = flipSprite,
                    .setCameraPosition = setCameraPosition,
                    .setZBuffer = setZBuffer,
                    .line = line,
                    .rectangle = rectangle,
                    .filledRectangle = filledRectangle,
                    .circle = circle,
                    .filledCircle = filledCircle,
                    .triangle = triangle,
                    .filledTriangle = filledTriangle};
