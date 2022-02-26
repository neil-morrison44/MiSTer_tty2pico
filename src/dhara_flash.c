
#include <stdbool.h>
#include "dhara/map.h"
#include "dhara/nand.h"
#include "hardware/regs/addressmap.h"
#include <hardware/flash.h>

#define FLASH_SIZE (8 * 1024 * 1024)
#define FLASH_OFFSET (4 * 1024 * 1024)

static struct dhara_map map;
static const struct dhara_nand nand = {
    .log2_page_size = 9, /* 512 bytes */
    .log2_ppb = 12 - 9,  /* 4096 bytes */
    .num_blocks = (FLASH_SIZE - FLASH_OFFSET) / FLASH_BLOCK_SIZE,
};

static uint8_t journal_buf[512];
static uint8_t tmp_buf[512];

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
                     dhara_error_t *err) {
  flash_range_erase(FLASH_OFFSET + (b * 4096), 4096);
  if (err) *err = DHARA_E_NONE;
  return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data, dhara_error_t *err) {
  flash_range_program(FLASH_OFFSET + (p * 512), data, 512);
  if (err) *err = DHARA_E_NONE;
  return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset,
                    size_t length, uint8_t *data, dhara_error_t *err) {
  memcpy(
      data,
      (uint8_t *)XIP_NOCACHE_NOALLOC_BASE + FLASH_OFFSET + (p * 512) + offset,
      length);
  if (err) *err = DHARA_E_NONE;
  return 0;
}

int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b) { return 0; }

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b) {}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p) {
  dhara_error_t err = DHARA_E_NONE;

  dhara_nand_read(&nand, p, 0, 512, tmp_buf, &err);
  if (err != DHARA_E_NONE) return 0;
  for (int i = 0; i < 512; i++)
    if (tmp_buf[i] != 0xff) return 0;
  return 1;
}

int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src,
                    dhara_page_t dst, dhara_error_t *err) {
  dhara_nand_read(&nand, src, 0, 512, tmp_buf, err);
  if (*err != DHARA_E_NONE) return -1;

  return dhara_nand_prog(&nand, dst, tmp_buf, err);
}

void flash_dev_init(void) {
  printf("NAND flash, ");
  dhara_map_init(&map, &nand, journal_buf, 10);
  dhara_error_t err = DHARA_E_NONE;
  dhara_map_resume(&map, &err);
  uint32_t lba = dhara_map_capacity(&map);
  printf("%dkB physical %dkB logical at 0x%p: ", nand.num_blocks * 4, lba / 2,
         XIP_NOCACHE_NOALLOC_BASE + FLASH_OFFSET);
}
