#include "am.h"
#include "memory.h"
#include <loader.h>
#include <debug.h>
#include <fs.h>

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
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  assert(filename);
  int fd = fs_open(filename, 0, 0);

  int rc = 0;
  Elf_Ehdr elf_header;
  fs_lseek(fd, 0, SEEK_SET);
  rc = fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
  assert(rc == sizeof(Elf_Ehdr));

  Log("Loading '%s'", filename);

  assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);
  assert(elf_header.e_machine == EXPECT_TYPE);

  uintptr_t phoff = elf_header.e_phoff;
  uint32_t phnum = elf_header.e_phnum;
  uint32_t phentsize = elf_header.e_phentsize;

  Elf_Phdr entry;
  for (uint32_t i = 0; i < phnum; i ++) {
    uintptr_t entoff = phoff + i * phentsize;
    fs_lseek(fd, entoff, SEEK_SET);
    rc = fs_read(fd, &entry, sizeof(Elf_Phdr));
    assert(rc == sizeof(Elf_Phdr));

    if (entry.p_type != PT_LOAD) continue;

    fs_lseek(fd, entry.p_offset, SEEK_SET);
    rc = fs_read(fd, (void *)entry.p_vaddr, entry.p_filesz);
    assert(rc == entry.p_filesz);

    memset((void *)(entry.p_vaddr + entry.p_filesz), 0, entry.p_memsz - entry.p_filesz);
  }

  Log("Loaded, entry at %p.", filename, elf_header.e_entry);

  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void (*func)(void *), void *arg) {
  Area kstack = { .start = pcb->stack, .end = pcb->stack + STACK_SIZE };
  pcb->cp = kcontext(kstack, func, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  Area kstack = { .start = pcb->stack, .end = pcb->stack + STACK_SIZE };
  Area ustack = { .start = new_page(USTACK_NR_PAGES) };
  ustack.end = ustack.start + PGSIZE * USTACK_NR_PAGES;

  int argc = 0, envc = 0, strsize = 0;
  if (argv != NULL) {
    printf("T1\n");
    for (; argv[argc] != NULL; argc ++) {
      printf("argv[%d]: %s\n", argc, argv[argc]);
      strsize += strlen(argv[argc]) + 1;
    }
  }
  if (envp != NULL) {
    printf("T2\n");
    for (; envp[envc] != NULL; envc ++) {
      printf("envp[%d]: %s\n", envc, envp[envc]);
      strsize += strlen(envp[envc]) + 1;
    }
  }

  printf("argc: %d, envc: %d, strsize: %d.\n", argc, envc, strsize);

  void *strpointer = ustack.end - strsize * sizeof(char);

  void *pointer = strpointer
    - sizeof(int)
    - (argc + envc + 2) * sizeof(const char *);

  void *sp = pointer;

  printf("S0\n");

  *(int *)pointer = argc; pointer += sizeof(int);
  for (int i = 0; i < argc; i ++) {
    *(char **)pointer = strpointer;
    strcpy(strpointer, argv[i]);
    pointer += sizeof(char *);
    strpointer += (strlen(argv[i]) + 1) * sizeof(char);
  }

  printf("S1\n");

  *(char **)pointer = NULL; pointer += sizeof(char **);

  printf("S2\n");

  for (int i = 0; i < envc; i ++) {
    *(char **)pointer = strpointer;
    strcpy(strpointer, envp[i]);
    pointer += sizeof(char *);
    strpointer += (strlen(argv[i]) + 1) * sizeof(char);
  }
  *(char **)pointer = NULL; pointer += sizeof(char **);

  printf("S3\n");

  void *entry = (void *)loader(pcb, filename);
  AddrSpace as = { PGSIZE, kstack, NULL }; // TODO: not correct
  pcb->cp = ucontext(&as, kstack, entry);
  pcb->cp->GPRx = (uintptr_t) sp;
}
