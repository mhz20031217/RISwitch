#ifndef __LOADER_H__
#define __LOADER_H__

#include <common.h>
#include <proc.h>
#include <elf.h>

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*func)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename);
#endif
