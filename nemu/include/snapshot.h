#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include <common.h>

bool snapshot_load(const char *filename);
bool snapshot_save(const char *filename);

#endif // __SNAPSHOT_H__
