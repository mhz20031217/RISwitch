#include <common.hpp>
#include <memory.hpp>

static word_t mem[IMEM_SIZE/4];

void imem_load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  assert(fp);

  char buf[64];

  paddr_t i = 0;
  while (fgets(buf, 64, fp)) {
    // printf("buf: %s\n", buf);
    if (buf[0] == '@') {
      int rc = sscanf(buf + 1, "%x", &i);
      // printf("reset i at %x %d\n", i, i);
      assert(i%4 == 0);
      continue;
    }
    if (i >= IMEM_SIZE/4) {
      // printf("i error: %x %d\n", i ,i);
      assert(0);
    }
    int rc = sscanf(buf, "%x", &mem[i/4]);
    // printf("res: mem[%x(%d)] = %x(%d)", i/4, i/4, mem[i/4], mem[i/4]);
    assert(rc);
    i += 4;
  }
  fclose(fp);
}

word_t imem_read(const paddr_t addr) {
  assert(addr < IMEM_SIZE); // check bound
  assert((addr & 0x3U) == 0); // check instruction alignment
  printf("imem_read(%x) = %x(%d)\n", addr, mem[addr >> 2], mem[addr >> 2]);
  return mem[addr >> 2]; 
}

