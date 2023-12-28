#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include <common.hpp>

#define IMEM_SIZE 131072U // 128KB
#define DMEM_SIZE 131072U * 16 // 2048KB

void imem_load(const char *filename);

extern "C" void imem_read(paddr_t addr, word_t *data);

void dmem_load(const char *filename);

extern "C" void dmem_read(paddr_t addr, word_t *data);

extern "C" void dmem_write(paddr_t addr, word_t data, vluint8_t wmask);

#endif // !_MEMORY_HPP_
