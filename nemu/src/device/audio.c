/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "debug.h"
#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  reg_lock,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static bool audio_initialized = false;

static void audio_fill_buffer(void *data, Uint8 *stream, int len) {
  Info("Filling stream of length: %d.", len);

  int sum = 0;

  while (sum < len) {
    while (audio_base[reg_lock] || audio_base[reg_count] == 0) {
      usleep(10);
    }
    audio_base[reg_lock] = 1;
    Info("Acquired lock.");

    int count = audio_base[reg_count];

    if (count <= len - sum) {
      memcpy(stream + sum, sbuf, count);
      sum += count;
      audio_base[reg_count] = 0;
    } else {
      int size = len - sum;
      memcpy(stream + sum, sbuf, size);
      audio_base[reg_count] = count - size;
      memmove(sbuf, sbuf + size, count - size);
    }

    audio_base[reg_lock] = 0;
    Info("Released lock.");
    Info("Filled %d / %d", sum, len);
  }
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (offset == reg_init) {
    if (audio_initialized) {
      SDL_CloseAudio();
    }

    SDL_AudioSpec spec = {
        .freq = audio_base[reg_freq],
        .channels = audio_base[reg_channels],
        .silence = 0,
        .format = AUDIO_S16SYS,
        .samples = audio_base[reg_samples],
        .userdata = NULL,
        .callback = audio_fill_buffer
    };

    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);

    audio_initialized = true;
    
    Info("Audio initialized, freq: %d, channel: %d, samples: %d.",
      audio_base[reg_freq], audio_base[reg_channels], audio_base[reg_samples]);
    audio_base[reg_init] = 0;
  }
}

void init_audio() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  audio_base[reg_lock] = 0;
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
