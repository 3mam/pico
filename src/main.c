#include <emscripten.h>
#include <string.h>
#include "draw.h"
#include "str.h"

EM_JS(void, callgl, (i16 w, i16 h, void* p), {
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA,
                gl.UNSIGNED_SHORT_5_5_5_1, HEAPU16, p / 2);
  gl.drawArrays(gl.TRIANGLES, 0, 6);
});

EM_JS(void, print, (int a), { console.log('foo', a); });

typedef struct {
  int a;
  ui8 b : 2;
  ui16* video_buffer;
} foo;

#include "data.h"
void test(void* arg) {
  foo* f = (foo*)arg;
  draw.clearScreen(0);
  draw.setPixel(f->a, f->a, 1);

  f->a++;
  // print(&arg);
  // ui16* c = deb_canvas();
  // print(draw.pget(100, 100));

  draw.setCameraPosition((point){.x = f->b, .y = f->b});
  draw.setZBuffer(2);
  draw.flipSprite(f->b);
  draw.sprite(2, (point){.x = 10, .y = 10});
  draw.setZBuffer(0);

  // print(f->b);
  f->b++;
  draw.setCameraPosition((point){0, 0});

  draw.setColor(2);
  draw.filledRectangle((point){10, 10}, (point){100, 100});
  draw.setColor(1);
  draw.filledRectangle((point){5, 5}, (point){20, 20});
  draw.setColor(2);
  draw.triangle((point){100, 100}, (point){120, 120}, (point){50, 120});
  draw.updateVideoBuffer();
  callgl(200, 200, f->video_buffer);
  emscripten_async_call(test, arg, -1);
}

int main() {
  foo* f = NEW(foo);
  f->a = 10;
  f->b = 0;
  ui16* video_buffer = MAKE(ui16, 200 * 200);
  f->video_buffer = video_buffer;
  draw.init(200, 200, video_buffer);

  test(f);
  return 0;
}