#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include <common.hpp>

#define IMEM_SIZE 131072 // 128KB
#define DMEM_SIZE 131072U // 128KB

#define M_X 0U
#define M_LB 0U
#define M_LH 1U
#define M_LW 2U
#define M_LBU 4U
#define M_LHU 5U
#define M_SB M_LB
#define M_SH M_LH
#define M_SW M_LW

void imem_load(const char *filename);

word_t imem_read(const paddr_t addr);

void dmem_load(const char *filename);

word_t dmem_read(paddr_t addr, vluint8_t memOp);

void dmem_write(paddr_t addr, vluint8_t memOp, word_t din);

#endif // !_MEMORY_HPP_
