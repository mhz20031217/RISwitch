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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, NR_SPECIAL_FILES};

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

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
  [FD_FB] = {"fb", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define NR_FILES ARRLEN(file_table)

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // for (size_t i = NR_SPECIAL_FILES; i < NR_FILES; i ++) {
  //   Finfo *file = &file_table[i];
  //   file->read = fs_read;
  //   file->write = fs_write;
  // }
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
  
  size_t rc = min(len, file->size - file->offset);
  if (rc != ramdisk_read(buf, file->disk_offset + file->offset, rc)) {
    return -1;
  }

  file->offset += rc;
  return rc;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  Finfo *file = check_fd(fd);
  
  size_t rc = min(len, file->size - file->offset);
  if (rc != ramdisk_write(buf, file->disk_offset + file->offset, rc)) {
    return -1;
  }

  file->offset += rc;
  return rc;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo *file = check_fd(fd);

  size_t pos;
  switch (whence) {
    case SEEK_CUR: pos = file->offset + offset; break;
    case SEEK_SET: pos = offset; break;
    case SEEK_END: pos = file->size; break;
    default: panic("fs: Unsupported seek option: %d.", whence);
  }
  if (pos > file->size) {
    Log("fs: File offset out of bound: file %d at offset %lu.", fd, pos);
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
