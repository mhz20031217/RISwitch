#include <NDL.h>
#include <SDL.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>

// hack
#include <sdl-audio.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define NR_KEYS ARRLEN(keyname)

static uint8_t keystate[NR_KEYS];

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

static int identify_key(char buf[], SDL_Event *ev) {
  SDL_AudioCallback();
  if (buf[0] != 'k') {
    Error("spec error: begins with space.");
    return 0;
  }

  for (int i = 3; i < 64; i ++) {
    if (buf[i] == '\n') {
      buf[i] = '\0';
      break;
    }
  }

  int i;
  for (int i = 0; i < NR_KEYS; i ++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      ev->key.keysym.sym = i;
    }
  }

  if (i == NR_KEYS) {
    Error("no such key: %d.", i);
  }

  if (buf[1] == 'd') {
    keystate[ev->key.keysym.sym] = 1;
    ev->type = SDL_KEYDOWN;
  } else {
    keystate[ev->key.keysym.sym] = 0;
    ev->type = SDL_KEYUP;
  }
  // printf("%s\n", buf);
  return 1;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf)) == 0) {
    return 0;
  }

  return identify_key(buf, ev);  
}

int SDL_WaitEvent(SDL_Event *ev) {
  char buf[64];
  while (NDL_PollEvent(buf, sizeof(buf)) == 0);

  return identify_key(buf, ev);
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  SDL_Event ev;
  while (SDL_PollEvent(&ev));
  if (numkeys) {
    *numkeys = NR_KEYS;
  }
  return keystate;
}
