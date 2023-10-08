#include <stdio.h>
#include <stdlib.h>
#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("[SDL_image] no such file or directory: '%s'.\n", filename);
    return NULL;
  }

  if (fseek(fp, 0, SEEK_END)) {
    printf("[SDL_image] cannot determine file size.\n");
  }

  long size = ftell(fp);

  void *buf = malloc(size);
  rewind(fp);

  fread(buf, size, 1, fp);

  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, size);

  free(buf);

  if (!ret)
    printf("[SDL_image] failed.\n");

  fclose(fp);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
