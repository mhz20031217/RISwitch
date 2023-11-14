#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include "sdl-audio.h"

static void (*callback)(void *userdata, uint8_t *stream, int len) = NULL;
static void *userdata = NULL;
static bool playing = false;
static int freq, channels, samples;
static bool callback_flag = false;

// 96000 * 2 * 4
static unsigned char sbuf[768000];

void SDL_AudioCallback() {
  printf("[SDL] Audio callback.\n");
  if (callback == NULL || !playing || callback_flag) {
    return;
  }
  callback_flag = true;

  int len = NDL_QueryAudio();
  printf("[SDL] Available count: %d.\n", len);
  if (len < channels * freq) {
    return;
  }

  callback(userdata, sbuf, len);
  printf("[SDL] Play length: %d.\n", len);
  NDL_PlayAudio(sbuf, len);
  callback_flag = false;
  printf("[SDL] Play end.\n");
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  callback_flag = false;
  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);

  callback = desired->callback;
  userdata = desired->userdata;

  if (obtained) {
    *obtained = *desired;
  }
  return 0;
}

void SDL_CloseAudio() {
  callback = NULL;
  userdata = NULL;
  NDL_CloseAudio();
}

void SDL_PauseAudio(int pause_on) {
  playing = !pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
}

void SDL_LockAudio() {
}

void SDL_UnlockAudio() {
}
