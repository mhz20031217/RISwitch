#ifndef _INSTRUCTIONS_HPP_
#define _INSTRUCTIONS_HPP_

#include <string>
#include <vector>

const std::vector<std::string> instructions = {
    "add", "addi", "and", "andi", "auipc", "beq",  "bge", "bgeu",
    "blt", "bltu", "bne", "jal",  "jalr",  "lb",   "lbu", "lh",
    "lhu", "lui",  "lw",  "or",   "ori",   "sb",   "sh",  "simple",
    "sll", "slli", "slt", "slti", "sltiu", "sltu", "sra", "srai",
    "srl", "srli", "sub", "sw",   "xor",   "xori"};

#endif // !_INSTRUCTIONS_HPP_
