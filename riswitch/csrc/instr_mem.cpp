#include <common.hpp>
#include <memory.hpp>

static word_t mem[IMEM_SIZE/4];

void imem_load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  assert(fp);

  char buf[64];

  paddr_t i = 0;
  while (fgets(buf, 64, fp)) {
    if (buf[0] == '@') {
      int rc = sscanf(buf + 1, "%x", &i);
      i %= IMEM_SIZE;
      continue;
    }
    if (i >= IMEM_SIZE/4) {
      assert(0);
    }
    int rc = sscanf(buf, "%x", &mem[i]);
    assert(rc);
    i ++;
  }
  fclose(fp);
}

extern "C" void imem_read(paddr_t addr, word_t *data) {
  *data = mem[((addr & ~0x3U) % IMEM_SIZE) >> 2];
}

