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
static int sbdev = -1;
static int sbctldev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int offset_w = 0, offset_h = 0;

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

    char buf[64], key[64];
    int value;
    int nread = read(disp_fd, buf, sizeof(buf) - 1);
    if (nread <= 0) {
      printf("[NDL] Cannot read '/proc/dispinfo'.\n");
      *w = *h = 0;
      return;
    }
    buf[nread] = '\0';

    int i = 0, sep = -1;
    for (int j = 0; j < nread; j ++) {
      if (buf[j] == ':') {
        sep = j;
        buf[j] = '\0';
      } else if (buf[j] == '\n') {
        sscanf(buf + i, "%s", key);
        sscanf(buf + sep + 1, "%d", &value);
        if (strcmp(key, "WIDTH") == 0) {
          screen_w = value;
        } else if (strcmp(key, "HEIGHT") == 0) {
          screen_h = value;
        } else {
          printf("[NDL] Invalid key in '/proc/dispinfo': '%s'.\n", key);
        }
        i = j + 1;
      }
    }
    

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

    offset_w = (screen_w - canvas_w) / 2;
    offset_h = (screen_h - canvas_h) / 2;

    fbdev = open("/dev/fb", O_SYNC);

    if (fbdev == -1) {
      printf("[NDL] Failed to open framebuffer.\n");
      canvas_w = canvas_h = *w = *h = 0;
      return;
    }

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
    lseek(fbdev, ((offset_h+y+i)*screen_w+x+offset_w)*4, SEEK_SET);
    (void) write(fbdev, pixels + w * i, w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  sbctldev =  open("/dev/sbctl", O_SYNC);
  if (sbctldev == -1) {
    printf("[NDL] Audio device unavailable.\n");
    return;
  }

  int args[] = { freq, channels, samples };
  if (write(sbctldev, args, 12) != 12) {
    printf("[NDL] Audio device initialization failed.\n");
    sbdev = -1;
    return;
  }

  sbdev = open("/dev/sb", O_SYNC);
  if (sbdev == -1) {
    printf("[NDL] Sound buffer unavailable.\n");
    return;
  }

  printf("[NDL] Audio initialized.\nfreq: freq = %d, channels = %d, samples = %d.\n", freq, channels, samples);
}

void NDL_CloseAudio() {
  if (sbdev != -1) close(sbdev);
  sbdev = -1;
}

int NDL_PlayAudio(void *buf, int len) {
  if (sbdev == -1) {
    printf("[NDL] Audio used uninitialized.\n");
    return 0;
  }

  return write(sbdev, buf, len);
}

int NDL_QueryAudio() {
  if (sbctldev == -1) {
    return 0;
  }
  
  int avail;
  read(sbctldev, &avail, 4);

  return avail;
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
  if (evtdev != -1) close(evtdev);
  if (fbdev != -1) close(fbdev);
  if (sbdev != -1) close(sbdev);
  if (sbctldev != -1) close(sbctldev);
  evtdev = fbdev = sbctldev = sbdev = -1;
}
