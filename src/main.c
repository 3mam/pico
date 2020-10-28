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
  draw.cls(0);
  draw.pset(f->a, f->a, 1);

  f->a++;
  // print(&arg);
  // ui16* c = deb_canvas();
  // print(draw.pget(100, 100));
  draw.camera(f->b, f->b);
  draw.zbuff(4);
  draw.flip(f->b);
  draw.spr(2, 10, 10);
  draw.zbuff(3);

  // print(f->b);
  f->b++;
  draw.camera(0, 0);

  draw.color(2);
  draw.rectf(10, 10, 100, 100);
  draw.color(1);
  draw.rectf(5, 5, 20, 20);
  draw.color(2);
  draw.trif(100, 100, 120, 120, 50, 120);
  draw.update();
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