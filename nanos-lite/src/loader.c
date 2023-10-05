#include <proc.h>
#include <elf.h>
#include <debug.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

// ISA-depedent definitions
#if defined(__ISA_X86__)
# define EXPECT_TYPE EM_386
#elif defined(__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86_64__)
# define EXPECT_TYPE EM_X86_64
#else
#error Unsupported ISA
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len); 

static uintptr_t loader(PCB *pcb, const char *filename) {
  int rc = 0;
  Elf_Ehdr elf_header;
  rc = ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
  assert(rc == sizeof(Elf_Ehdr));

  printf("Read elf done, elf_header.ident: 0x%x\n", elf_header.e_ident);

  assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);
  assert(elf_header.e_machine == EXPECT_TYPE);

  uintptr_t phoff = elf_header.e_phoff;
  uint32_t phnum = elf_header.e_phnum;
  uint32_t phentsize = elf_header.e_phentsize;

  Elf_Phdr entry;
  for (uint32_t i = 0; i < phnum; i ++) {
    uintptr_t entoff = phoff + i * phentsize;
    rc = ramdisk_read(&entry, entoff, sizeof(Elf_Phdr));
    assert(rc == sizeof(Elf_Phdr));

    if (entry.p_type != PT_LOAD) continue;

    rc = ramdisk_read((void *)entry.p_vaddr, entry.p_offset, entry.p_filesz);
    assert(rc == entry.p_filesz);

    memset((void *)(entry.p_vaddr + entry.p_filesz), 0, entry.p_memsz - entry.p_filesz);
  }

  return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

