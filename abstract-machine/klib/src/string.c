#include <klib.h>
#include <klib-macros.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (*s != '\0') {
    s ++;
    len ++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *cur = dst;
  while (*src != '\0') {
    *(cur ++) = *(src ++);
  }
  *cur = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t cnt = 0;
  char *cur = dst;

  while (cnt < n && (*src != '\0')) {
    *(cur ++) = *(src ++);
    cnt ++;
  }

  *cur = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *ret = dst;

  while (*dst != '\0') dst ++;

  while (*src != '\0') {
    *(dst ++) = *(src ++);
  }
  *dst = '\0';

  return ret;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
    s1 ++; s2 ++;
  }

  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t cnt = 0;

  while (cnt < n && *s1 != '\0' && *s2 != '\0') {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
    s1 ++; s2 ++;
  }

  if (cnt == n) {
    return 0;
  }

  return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
  unsigned char ch = c;

  unsigned char *cur = s;
  size_t cnt = 0;

  while (cnt < n) {
    *(cur ++) = ch;
    cnt ++;
  }

  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  const unsigned char *src = in;
  unsigned char *dst = out;
  size_t cnt = 0;

  while (cnt < n) {
    *(dst ++) = *(src ++);
    cnt ++;
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *c1 = s1, *c2 = s2;
  size_t cnt = 0;

  while (cnt < n) {
    if (*c1 != *c2) {
      return *c1 - *c2;
    }
    c1 ++; c2 ++;
    cnt ++;
  }

  return (unsigned char) 0;
}

#endif
