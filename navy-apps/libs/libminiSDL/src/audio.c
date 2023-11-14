#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sdl-audio.h"

#define min(a, b) (((a) < (b))?(a):(b))
#define max(a, b) (((a) > (b))?(a):(b))

static void (*callback)(void *userdata, uint8_t *stream, int len) = NULL;
static void *userdata = NULL;
static bool playing = false;
static int freq, channels, samples;
static bool callback_flag = false;

// 96000 * 2 * 4
#define SBUF_SIZE 768000
static unsigned char sbuf[SBUF_SIZE];

void SDL_AudioCallback() {
  printf("[SDL] Audio callback.\n");
  if (callback == NULL || !playing || callback_flag) {
    printf("[SDL] abort callback. callback: %p, playing: %d, callback_flag: %d.\n", callback, playing, callback_flag);
    return;
  }
  

  int len = NDL_QueryAudio();
  printf("[SDL] Available count: %d.\n", len);
  if (len < channels * freq) {
    return;
  }
  len = min(channels * freq, SBUF_SIZE);

  printf("[SDL] Play length: %d.\n", len);
  assert(sbuf);
  callback_flag = true;
  callback(userdata, sbuf, len);
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
  freq = desired->freq;
  channels = desired->channels;
  samples = desired->samples;

  // if (!sbuf) sbuf = malloc(SBUF_SIZE);
  
  return 0;
}

void SDL_CloseAudio() {
  callback = NULL;
  userdata = NULL;
  // if (sbuf) free(sbuf);
  // sbuf = NULL;
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
