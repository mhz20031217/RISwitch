#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  if (evtdev == -1) {
    printf("[NDL] Event device doesn't exist.\n");
    return 0;
  }
  size_t rc = read(evtdev, buf, len);
  if (rc == 0 || rc == -1) {
    return 0;
  }
  buf[rc] = '\0';
  return 1;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
    int disp_fd = open("/proc/dispinfo", O_SYNC);
    if (disp_fd == -1) {
      printf("[NDL] Cannot open '/proc/dispinfo'.\n");
      *w = *h = 0;
      return;
    }

    char buf[64], key[2][16];
    int value[2];
    int nread = read(disp_fd, buf, sizeof(buf) - 1);
    if (nread <= 0) {
      printf("[NDL] Cannot read '/proc/dispinfo'.\n");
      *w = *h = 0;
      return;
    }
    buf[nread] = '\0';

    sscanf(buf, "%s : %d \n%s : %d", key[0], &value[0], key[1], &value[1]);
    for (int i = 0; i < 2; i ++) {
      if (strcmp(key[i], "WIDTH") == 0) {
        screen_w = value[i];
      } else if (strcmp(key[i], "HEIGHT") == 0) {
        screen_h = value[i];
      } else {
        printf("[NDL] Invalid key in '/proc/dispinfo': '%s'.\n", key[i]);
      }
    }



    // int i;

    // for (i = 0; i < 100; i ++) {
    //   if ('0' <= buf[i] && buf[i] <= '9') {
    //     screen_w = 0;
    //     while (i < 100 && '0' <= buf[i] && buf[i] <= '9') {
    //       screen_w = screen_w * 10 + buf[i] - '0';
    //       i ++;
    //     }
    //     break;
    //   }
    // }

    // for (; i < 100; i ++) {
    //   if ('0' <= buf[i] && buf[i] <= '9') {
    //     screen_h = 0;
    //     while (i < 100 && '0' <= buf[i] && buf[i] <= '9') {
    //       screen_h = screen_h * 10 + buf[i] - '0';
    //       i ++;
    //     }
    //     break;
    //   }
    // }

    if (*w == 0 && *h == 0) {
      canvas_w = *w = screen_w;
      canvas_h = *h = screen_h;
    } else {
      if (*w > screen_w || *h > screen_h) {
        printf("[NDL] Canvas too large: %dx%d exceeds %dx%d.\n", *w, *h, screen_w, screen_h);
        *w = *h = 0;
        return;
      }
      canvas_w = *w;
      canvas_h = *h;
    }

    fbdev = open("/dev/fb", O_SYNC);

    printf("[NDL] Canvas open successfully, %dx%d (on %dx%d screen).\n",
           canvas_w, canvas_h, screen_w, screen_h);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  if (fbdev == -1 || canvas_w == 0 || canvas_h == 0) {
    printf("[NDL] No canvas.\n");
    return;
  }

  for (int i = 0; i < h; i ++) {
    lseek(fbdev, ((y+i)*screen_w+x)*4, SEEK_SET);
    write(fbdev, pixels + w * i, w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  } else {
    evtdev = open("/dev/events", O_SYNC);
  }

  return 0;
}

void NDL_Quit() {
}
