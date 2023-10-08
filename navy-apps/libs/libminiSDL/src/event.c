#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define NR_KEYS ARRLEN(keyname)

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[20];
  if (NDL_PollEvent(buf, 20) == 0) {
    return 0;
  }

  printf("SDL_PollEvent: %s\n", buf);

  if (buf[0] != 'k') {
    printf("[NDL] Spec error: begins with space.\n");
    return 0;
  }

  for (int i = 0; i < NR_KEYS; i ++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      ev->key.keysym.sym = i;
    }
  }

  if (buf[1] == 'd') {
    ev->type = SDL_KEYDOWN;
  } else {
    ev->type = SDL_KEYUP;
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
