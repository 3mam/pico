#ifndef DRAW_H
#define DRAW_H
#include "macro.h"

extern ui16* deb_canvas();

struct draw {
  /**
   * initialize only one instance for drawing runtine
   * @param width,height set size of 2D canvas
   * @param draw_call function called by update
   */
  void (*init)(i16 width, i16 height, void* video_buffer);
  void (*update)(void);

  /**
   * set color for objects
   * @param c requires value from 0 to 63
   */
  void (*color)(ui8 c);
  /**
   * draw pixel
   * @param x,y 2d coordinates
   */
  void (*pset)(i16 x, i16 y, ui8 c);
  /**
   * get pixel from canvas
   * @param x,y 2d coordinates
   * @return return color number.
   * If return 0 then no color found
   */
  ui8 (*pget)(i16 x, i16 y);
  /**
   * reset color palette to default
   */
  void (*palr)(void);
  /**
   * set transparency for colour
   * @param c color that requires value from 0 to 31
   * @param t if transparency is TRUE then color in sprite disappear
   */
  void (*palt)(ui8 c, bool t);
  /**
   * swaps palette
   * @param p from 0 to 7
   */
  void (*pal)(ui8 p);

  /**
   * deep buffer for drawing object
   * @param z requires value from 0 to 7.
   */
  void (*zbuff)(ui8 z);
  /**
   * clear canvas and set z buffer on 3
   * @param col set color with canvas that will by clear
   */
  void (*cls)(ui8 col);
  /**
   * draw sprite from data struct
   * @param n number of sprite to draw
   * @param x,y 2D coordinates
   */
  void (*spr)(ui16 n, i16 x, i16 y);
  /**
   * flip sprite
   * @param f
   * 0 standart position
   * 1 flip horizontal
   * 2 flip horizontal and vertical
   * 3 flip vertica
   */
  void (*flip)(ui8 f);
  /**
   * only draws in clipping
   */
  void (*clip)(i16 x, i16 y, i16 w, i16 h);
  /**
   * reset clipping to default screen size
   */
  void (*clipr)(void);
  /**
   * changes the screen view position
   */
  void (*camera)(i16 x, i16 y);
  void (*line)(i16 x0, i16 y0, i16 x1, i16 y1);
  void (*rect)(i16 x0, i16 y0, i16 x1, i16 y1);
  void (*rectf)(i16 x0, i16 y0, i16 x1, i16 y1);
  void (*circ)(i16 x0, i16 y0, i16 r);
  void (*circf)(i16 x0, i16 y0, i16 r);
  void (*tri)(i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2);
  void (*trif)(i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2);
};

extern struct draw draw;

#endif  // DRAW_H