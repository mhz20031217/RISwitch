#ifndef __DEBUG_H__
#define __DEBUG_H__

#define Log(format, ...) \
  printf("\33[1;36m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#undef Error
#define Error(format, ...) \
  do { \
    Log("\33[1;31mSDL critcal: " format, ## __VA_ARGS__); \
  } while (0)

#ifdef Assert
# undef Assert
#endif

#define Assert(cond) \
  do { \
    if (!(cond)) { \
      Error("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define TODO() panic("please implement me")

#endif
