#include <common.hpp>
#include <iomanip>
#include <memory.hpp>

static word_t mem[DMEM_SIZE/4];

static inline bool check_bound(paddr_t addr) {
  return addr < DMEM_SIZE;
}

void dmem_load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  assert(fp);

  char buf[64];

  paddr_t i = 0;
  while (fgets(buf, 64, fp)) {
    if (buf[0] == '@') {
      int rc = sscanf(buf + 1, "%x", &i);
      assert(i%4 == 0);
      i = i % DMEM_SIZE;
      continue;
    }
    if (i >= IMEM_SIZE/4) {
      assert(0);
    }
    int rc = sscanf(buf, "%x", &mem[i/4]);
    assert(rc);
    i += 4;
  }
  fclose(fp);
}

extern "C" void dmem_read(paddr_t addr, word_t *data) {
  *data = mem[((addr & ~0x3U) % DMEM_SIZE) >> 2];
  
  if (addr == 0x2002) {
    printf("0x2002 (mem[0x800]) is %x.\n", *data);
  }
}

extern "C" void dmem_write(paddr_t addr, word_t data, vluint8_t wmask) {
  size_t index = (addr & ~0x3U) % DMEM_SIZE;
  
  word_t mask = 0;
  if (wmask & 0x1U) mask |= 0xffU;
  if (wmask & 0x2U) mask |= 0xff00U;
  if (wmask & 0x4U) mask |= 0xff0000U;
  if (wmask & 0x8U) mask |= 0xff000000U;

  word_t x = mem[index >> 2] & ~mask;
  x |= mask & data;
  // debug
  if (addr == 0x2002) {
    printf("0x2002 (mem[0x800]) changed from %x to %x.\n", mem[index >> 2], x);
  }
  mem[index >> 2] = x;

}

