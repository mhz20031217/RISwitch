#include <SDL_bmp.h>
#include <BMP.h>
#include <assert.h>
#include <stdio.h>

SDL_Surface* SDL_LoadBMP(const char *filename) {
  printf("[SDL_bmp] Loading '%s'.\n", filename);
  int w, h;
  void *pixels = BMP_Load(filename, &w, &h);
  printf("[SDL_bmp] BMP_Load finished: %dx%d.\n", w, h);
  assert(pixels);
  SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, w * sizeof(uint32_t),
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
  s->flags &= ~SDL_PREALLOC;
  return s;
}
