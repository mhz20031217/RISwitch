#ifndef _COMMON_H_
#define _COMMON_H_

#include <verilated.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <svdpi.h>
#include <vector>
#include <set>

#define XLEN 32

#if XLEN == 32
typedef vluint32_t word_t;
typedef word_t paddr_t;
typedef word_t vaddr_t;
#else
typedef vluint64_t word_t;
typedef word_t paddr_t;
typedef word_t vaddr_t;
#endif

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

#define ROUNDUP(a, sz)   ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz) - 1))

#endif // !_COMMON_H_
