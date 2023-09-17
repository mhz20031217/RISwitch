/***************************************************************************************
* Copyright (c) 2023 Hongzhi Mao, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <elf.h>
#include "debug.h"
#include "macro.h"
#include <common.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Elf32_Ehdr elf_header;
static Elf32_Shdr *elf_section_table;
static Elf32_Sym *elf_symbol_table;
static char *elf_string_table;
static struct Func_info {
    uint32_t name_offset;
    Elf32_Addr value;
    uint32_t size;
} *func_table;
int nr_func_table = 0;

static const char *shift = "  ";
static int indent = 0;
static bool ftrace_initialized = 0;

void init_ftrace(const char *elf_file) {
    FILE *fp;

    if (elf_file != NULL) {
        Log("Ftrace enabled, loading ELF file: '%s'.", elf_file);
        fp = fopen(elf_file, "rb");
        Assert(fp, "Can not open '%s'", elf_file);
    } else {
        fp = NULL;
        Log("Ftrace disabled for no ELF file given.");
        return;
    }

    int rc;
    rc = fread(&elf_header, sizeof(Elf32_Ehdr), 1, fp);
    Assert(rc, "Failed to read elf_header.");
    rc = fseek(fp, elf_header.e_shoff, SEEK_SET);
    Assert(rc == 0, "Invalid ELF file.");

    elf_section_table = calloc(elf_header.e_shnum, elf_header.e_shentsize);
    Assert(elf_section_table, "Memory error. Failed to allocate memory for elf_section_table.");
    rc = fread(elf_section_table, elf_header.e_shentsize, elf_header.e_shnum, fp);
    Assert(rc, "Failed to read elf_section_table.");

    Elf32_Shdr *elf_symbol_table_section, *elf_string_table_section;

    elf_string_table_section = elf_symbol_table_section = NULL;

    for (int i = 0; i < elf_header.e_shnum; i ++) {
        if (elf_section_table[i].sh_type == SHT_SYMTAB) {
            elf_symbol_table_section = &elf_section_table[i];
        } else if (elf_section_table[i].sh_type == SHT_STRTAB) {
            elf_string_table_section = &elf_section_table[i];
        }
        if (elf_string_table_section && elf_string_table_section) break;
    }

    elf_symbol_table = malloc(elf_symbol_table_section->sh_size);
    Assert(elf_symbol_table, "Memory error. Failed to allocate memory for elf_symbol_table.");
    rc = fseek(fp, elf_symbol_table_section->sh_offset, SEEK_SET);
    Assert(rc == 0, "Invalid ELF file.");
    rc = fread(elf_symbol_table, elf_symbol_table_section->sh_size, 1, fp);
    Assert(rc, "Failed to read elf_symbol_table.");

    elf_string_table = malloc(elf_string_table_section->sh_size);
    Assert(elf_string_table, "Memory error. Failed to allocate memory for elf_string_table.");
    rc = fseek(fp, elf_string_table_section->sh_offset, SEEK_SET);
    Assert(rc == 0, "Invalid ELF file.");
    rc = fread(elf_string_table, elf_string_table_section->sh_size, 1, fp);
    Assert(rc, "Failed to read elf_string_table_section.");

    nr_func_table = 0;
    const int nr_symbol = elf_symbol_table_section->sh_size / elf_symbol_table_section->sh_entsize;
    for (int i = 0; i < nr_symbol; i ++) {
        if (ELF32_ST_TYPE(elf_symbol_table[i].st_info) == STT_FUNC) {
            nr_func_table ++;
        }
    }
    func_table = calloc(nr_func_table, sizeof(struct Func_info));
    Assert(func_table, "Memory error. Failed to allocate memory for func_table.");

    nr_func_table = 0;
    for (int i = 0; i < nr_symbol; i ++) {
        if (ELF32_ST_TYPE(elf_symbol_table[i].st_info) == STT_FUNC) {
            func_table[nr_func_table].name_offset = elf_symbol_table[i].st_name;
            func_table[nr_func_table].value = elf_symbol_table[i].st_value;
            func_table[nr_func_table].size = elf_symbol_table[i].st_size;
            nr_func_table ++;
        }
    }
    
    indent = 0;
    fclose(fp);
    ftrace_initialized = 1;
}

void destruct_ftrace() {
    if (elf_section_table) free(elf_section_table);
    if (elf_symbol_table) free(elf_symbol_table);
    if (elf_string_table) free(elf_string_table);
    if (func_table) free(func_table);
}

void ftrace(Elf32_Addr pc, Elf32_Addr addr, bool mode) {
    if (!ftrace_initialized) return;

    Info_n("0x%u: ", pc);
    for (int i = 0; i < indent; i ++) {
        Info_n("%s", shift);
    }
    
    char *func_name = NULL;
    for (int i = 0; i < nr_func_table; i ++) {
        struct Func_info *entry = &func_table[i];
        // if (entry->value <= addr && addr < entry->value + entry->size) {
        if (entry->value == addr) {
            func_name = elf_string_table + entry->name_offset;
            break;
        }
    }

    if (mode) {
        Info_n("call [%s@0x%u]\n", func_name, addr);
        indent ++;
    } else {
        Info_n("ret [%s]\n", func_name);
        if (indent > 0) indent --;
    }
}

#ifdef CONFIG_FTRACE
void ftrace_call(Elf32_Addr pc, Elf32_Addr addr) {
    ftrace(pc, addr, 1);
}

void ftrace_ret(Elf32_Addr pc, Elf32_Addr addr) {
    ftrace(pc, addr, 0);
}
#endif
