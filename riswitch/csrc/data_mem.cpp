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
    // printf("buf: %s", buf);
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
    // printf("res: mem[%x(%d)] = %x(%d)\n", i/4, i/4, mem[i/4], mem[i/4]);
    assert(rc);
    i += 4;
  }
  fclose(fp);
}

word_t dmem_read(paddr_t addr, vluint8_t memOp) {
  if (!check_bound(addr)) {
    std::cerr << "DMem out_of_bound: " << std::setbase(16) << addr << '\n';
  }

  addr %= DMEM_SIZE;

  // check addr alignment
  if (memOp == M_LW) {
    assert((addr & 0x3U) == 0);
  } else if (memOp == M_LH || memOp == M_LHU) {
    assert((addr & 0x1U) == 0);
  }

  word_t x = mem[addr >> 2];
  int offset = addr & 0x3U;

  word_t ret;
  if (memOp == M_LW) {
    ret = x;
  } else if (memOp == M_LHU || memOp == M_LH) {
    ret = (offset) ? BITS(x, 31, 16) : BITS(x, 15, 0);
    if (memOp == M_LH)
      ret = SEXT(ret, 16);
  } else if (memOp == M_LBU || memOp == M_LB) {
    if (offset == 0) {
      ret = BITS(x, 7, 0);
    } else if (offset == 1) {
      ret = BITS(x, 15, 8);
    } else if (offset == 2) {
      ret = BITS(x, 23, 16);
    } else if (offset == 3) {
      ret = BITS(x, 31, 24);
    } else {
      assert(0);
    }
    if (memOp == M_LB) {
      ret = SEXT(ret, 8);
    }
  } else {
    assert(0);
  }
  printf("dmem_read(%x, %d) = %x (%d)\n", addr, (int)memOp, ret, ret);
  return ret;
}

void dmem_write(paddr_t addr, vluint8_t memOp, word_t din) {
  printf("dmem_write(%x, %d, %x)\n", addr, (int)memOp, din);
  if (!check_bound(addr)) {
    std::cerr << "DMem out_of_bound: " << std::setbase(16) << addr << '\n';
  }

  addr %= DMEM_SIZE;

  // check addr alignment
  if (memOp == M_SW) {
    assert((addr & 0x3U) == 0);
  } else if (memOp == M_SH) {
    assert((addr & 0x1U) == 0);
  }


  word_t x = mem[addr >> 2];
  int offset = addr & 0x3U;

  if (memOp == M_SW) {
    x = din;
  } else if (memOp == M_SH) {
    if (offset == 2) {
      x = BITS(x, 15, 0) | (BITS(din, 15, 0) << 16);
    } else if (offset == 0){
      x = (BITS(x, 31, 16) << 16) | BITS(din, 15, 0);
    } else {
      assert(0);
    }
  } else if (memOp == M_SB) {
    if (offset == 0) {
      x = (BITS(x, 31, 8) << 8) | BITS(din, 7, 0);
    } else if (offset == 1) {
      x = (BITS(x, 31, 16) << 16) | (BITS(din, 7, 0) << 8) | BITS(x, 7, 0);
    } else if (offset == 2) {
      x = (BITS(x, 31, 24) << 24) | (BITS(din, 7, 0) << 16) | BITS(x, 15, 0);
    } else if (offset == 3) {
      x = (BITS(din, 7, 0) << 24) | BITS(x, 23, 0);
    } else {
      assert(0);
    }
  } else {
    assert(0);
  }

  mem[addr >> 2] = x;
}


