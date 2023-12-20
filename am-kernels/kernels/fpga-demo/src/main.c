#include <am.h>
#include <klib.h>
#include <klib-macros.h>

enum Color {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7
};

char to_ascii(int am_keycode);

AM_SWITCH_T sw;
AM_INPUT_KEYBRD_T kbd;

int screen_w, screen_h;

#define MAXW 80
#define MAXH 40

char screen[MAXW][MAXH];
uint8_t fg_color[MAXW][MAXH], bg_color[MAXW][MAXH];
int pos_x = 0, pos_y = 0;
char in_ascii = '\0';

void update_status_line() {
    int now = io_read(AM_TIMER_UPTIME).us / 1000000;
    assert(screen_w > 20 && screen_h > 10);
    for (int x = screen_w - 1; x >= 0 && now; x --) {
        screen[x][screen_h - 1] = now % 10 + '0';
        now /= 10;
    }

    for (int i = 15; i >= 0; i --) {
        bg_color[15 - i][screen_h - 1] = (sw.value & (1 << i)) ? WHITE : BLACK;
    }
}

void update_text() {
    if (kbd.keycode == AM_KEY_NONE) return;
    if (!kbd.keydown) return;
    char ascii = to_ascii(kbd.keycode);
    if (ascii != '\0') {
        screen[pos_x][pos_y] = ascii;
        if (pos_x == screen_w - 1) {
            if (pos_y == screen_h - 1) {
                pos_x = 0; pos_y = 0;
            } else {
                pos_y ++; pos_x = 0;
            }
        } else {
            pos_x ++;
        }
    }
}

void update_device() {
    sw = io_read(AM_SWITCH);
    kbd = io_read(AM_INPUT_KEYBRD);
    io_write(AM_LED, in_ascii & sw.value);
    io_write(AM_SEG, in_ascii);

    static uint64_t last_time = 0;
    uint64_t now = io_read(AM_TIMER_UPTIME).us;
    if (last_time == 0) {
        last_time = now;
    } else if (now - last_time < 10000) {
        return;
    } else {
        last_time = now;
    }

    // following code are updated every 1/100 s
    for (int i = 0; i < screen_w; i ++) {
        for (int j = 0; j < screen_h; j ++) {
            io_write(AM_CMEM_PUTCH, i, j, screen[i][j], fg_color[i][j], bg_color[i][j]);
        }
    }
}

int main() {
    io_write(AM_VGACTRL, 1);
    memset(screen, 0, sizeof(screen));
    memset(fg_color, 0xff, sizeof(fg_color));
    memset(bg_color, 0, sizeof(bg_color));
    AM_CMEM_CONFIG_T cmem_conf = io_read(AM_CMEM_CONFIG);
    AM_INPUT_CONFIG_T input_conf = io_read(AM_INPUT_CONFIG);
    assert(cmem_conf.present);
    assert(input_conf.present);
    screen_w = cmem_conf.width;
    screen_h = cmem_conf.height;
    while (true) {
        update_device();
        update_status_line();
        update_text();
    }
}
