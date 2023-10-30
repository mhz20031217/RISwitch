#include <debug.h>
#include <fs.h>
#include <common.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_FBCTL, FD_FB, FD_DISPINFO, NR_SPECIAL_FILES};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_FBCTL] = {"/dev/fbctl", 0, 0, invalid_read, invalid_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

#define NR_FILES ARRLEN(file_table)

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  char buf[100];
  if (gpu_config.present) {
    file_table[FD_FB].size = gpu_config.width * gpu_config.height * 4;
    snprintf(buf, 100, "WIDTH : %d\nHEIGHT : %d\n", gpu_config.width, gpu_config.height);
    file_table[FD_DISPINFO].size = strlen(buf);
  }
}

int fs_open(const char *pathname, int flags, int mode) {
  
  for (int i = 0; i < NR_FILES; i ++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      return i;
    }
  }
  panic("fs: No such file or directory: %s", pathname);
}

static inline Finfo *check_fd(int fd) {
  assert(0 <= fd && fd < NR_FILES);
  return &file_table[fd];
}

size_t fs_read(int fd, void *buf, size_t len) {
  Finfo *file = check_fd(fd);
  
  if (file->read == NULL) {
    size_t rc = min(len, file->size - file->offset);
    if (rc != ramdisk_read(buf, file->disk_offset + file->offset, rc)) {
      return -1;
    }
    file->offset += rc;
    return rc;
  } else {
    size_t rc;
    rc = file->read(buf, file->offset, len);
    file->offset += rc;
    return rc;
  }

}

size_t fs_write(int fd, const void *buf, size_t len) {
  Finfo *file = check_fd(fd);
  
  if (file->write == NULL) {
    size_t rc = min(len, file->size - file->offset);
    if (rc != ramdisk_write(buf, file->disk_offset + file->offset, rc)) {
      return -1;
    }
    file->offset += rc;
    return rc;
  } else {
    size_t rc;
    rc = file->write((void *)buf, file->offset, len);
    if (rc == -1) {
      // Log("Impossible...");
      return -1;
    }
    file->offset += rc;
    return rc;
  }
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo *file = check_fd(fd);

  size_t pos;
  switch (whence) {
    case SEEK_CUR: pos = file->offset + offset; break;
    case SEEK_SET: pos = offset; break;
    case SEEK_END: pos = file->size; break;
    default: panic("Unsupported seek option: %d.", whence);
  }
  if (pos > file->size) {
    Log("File offset out of bound: file %d at offset %lu.", fd, pos);
    return -1;
  }

  file->offset = pos;
  return pos;
}

int fs_close(int fd) {
  return 0;
}

const char *fs_getfilename(int fd) {
  Finfo *file = check_fd(fd);
  return file->name;
}
