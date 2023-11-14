#include <isa.h>
#include <common.h>
#include <cpu/cpu.h>
#include <cpu/difftest.h>
#include <memory/paddr.h>

bool snapshot_load(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp)
    goto error;

  int rc = fread(&cpu, 1, sizeof(cpu), fp);
  if (rc != sizeof(cpu)) goto error;
  rc = fread(guest_to_host(RESET_VECTOR), 1, CONFIG_MSIZE, fp);
  if (rc != CONFIG_MSIZE) goto error;

  difftest_sync();
  return true;
error:
  if (fp)
    fclose(fp);
  return false;
}

bool snapshot_save(const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (!fp)
    goto error;

  int rc = fwrite(&cpu, 1, sizeof(cpu), fp);
  if (rc != sizeof(cpu)) goto error;
  rc = fwrite(guest_to_host(RESET_VECTOR), 1, CONFIG_MSIZE, fp);
  if (rc != CONFIG_MSIZE) goto error;

  return true;
error:
  if (fp)
    fclose(fp);
  return false;
}
