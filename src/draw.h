#ifndef DRAW_H
#define DRAW_H
#include "macro.h"

extern ui16* deb_canvas();

typedef struct {
  i16 x;
  i16 y;
} point;

typedef struct {
  i16 x;
  i16 y;
  i16 width;
  i16 height;
} reactangle;

struct draw {
  /**
   * initialize only one instance for drawing runtine
   * @param width,height set size of 2D canvas
   * @param draw_call function called by update
   */
  void (*init)(i16 width, i16 height, void* video_buffer);
  void (*updateVideoBuffer)(void);

  /**
   * set color for objects
   * @param c requires value from 0 to 31
   */
  void (*setColor)(ui8 color);
  /**
   * draw pixel
   * @param x,y 2d coordinates
   */
  void (*setPixel)(i16 x, i16 y, ui8 color);
  /**
   * get pixel from canvas
   * @param x,y 2d coordinates
   * @return return color number.
   * If return 0 then no color found
   */
  ui8 (*getPixel)(i16 x, i16 y);
  /**
   * reset color palette to default
   */
  void (*resetColorsTransparent)(void);
  /**
   * set transparency for colour
   * @param color color that requires value from 0 to 31
   * @param transparent if transparency is TRUE then color in sprite disappear
   */
  void (*setColorToTransparent)(ui8 color, bool transparent);
  /**
   * swaps palette
   * @param pallet_number from 0 to 7
   */
  void (*swapPallet)(ui8 pallet_number);

  /**
   * deep buffer for drawing object
   * @param z requires value from -3 to 3.
   */
  void (*setZBuffer)(i8 value);
  /**
   * clear canvas and set z buffer on 3
   * @param color set color with canvas that will by clear
   */
  void (*clearScreen)(ui8 color);
  /**
   * draw sprite from data struct
   * @param n number of sprite to draw
   * @param x,y 2D coordinates
   */
  void (*sprite)(ui16 number, point position);
  /**
   * flip sprite
   * @param position
   * 0 standart position
   * 1 flip horizontal
   * 2 flip horizontal and vertical
   * 3 flip vertica
   */
  void (*flipSprite)(ui8 position);
  /**
   * only draws in clipping
   */
  void (*clipping)(reactangle dimensions);
  /**
   * reset clipping to default screen size
   */
  void (*resetClipping)();
  /**
   * changes the screen view position
   */
  void (*setCameraPosition)(point position);
  void (*line)(point first, point second);
  void (*rectangle)(point first, point second);
  void (*filledRectangle)(point first, point second);
  void (*circle)(point position, i16 radius);
  void (*filledCircle)(point position, i16 radius);
  void (*triangle)(point first, point second, point third);
  void (*filledTriangle)(point first, point second, point third);
};

extern struct draw draw;

#endif  // DRAW_H