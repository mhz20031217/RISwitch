#include <NDL.h>
#include <SDL.h>
#include "sdl-audio.h"

static void (*callback)(void *userdata, uint8_t *stream, int len) = NULL;
static void *userdata = NULL;
static bool playing = false;
static int freq, channels, samples;

// 96000 * 2 * 4
static unsigned char sbuf[768000];

void SDL_AudioCallback() {
  printf("[SDL] Audio callback.\n");
  if (callback == NULL || !playing) {
    return;
  }

  int len = NDL_QueryAudio();
  if (len < channels * freq) {
    return;
  }

  callback(userdata, sbuf, len);
  NDL_PlayAudio(sbuf, len);
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
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
