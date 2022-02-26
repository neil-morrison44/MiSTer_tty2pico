#include "ff.h"
#include "stdio.h"
#include "string.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "dhara/map.h"

#ifdef IO_FLASH

// https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__flash.html

// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (256 * 1024)
const uint8_t* flash_target_contents =
    (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);

void eraseFlash() {
  printf("\nErasing target region...\n");
  flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
}

DSTATUS disk_status(BYTE pdrv /* [IN] Physical drive number */
) {
  printf("disk_status \n");
  return 0;
}

DSTATUS disk_initialize(BYTE pdrv /* [IN] Physical drive number */
) {
  // uint8_t flashId;
  // flash_get_unique_id(&flashId);
  // printf("disk_initialize, ID: %i \n", flashId);
  flash_dev_init();
  return 0;
};

DRESULT disk_read(BYTE pdrv,    /* [IN] Physical drive number */
                  BYTE* buff,   /* [OUT] Pointer to the read data buffer */
                  LBA_t sector, /* [IN] Start sector number */
                  UINT count    /* [IN] Number of sectros to read */
) {
  printf("disk read \n");
  int index = FLASH_SECTOR_SIZE * sector;
  int length = FLASH_SECTOR_SIZE * count;
  dhara_map_read(&map, sector, buff, &err);
  memcpy(&buff, &flash_target_contents[index], length * sizeof(uint8_t));
  return RES_OK;
};

DRESULT disk_write(
    BYTE pdrv,        /* [IN] Physical drive number */
    const BYTE* buff, /* [IN] Pointer to the data to be written */
    LBA_t sector,     /* [IN] Sector number to write from */
    UINT count        /* [IN] Number of sectors to write */
) {
  int index = FLASH_SECTOR_SIZE * sector;
  int length = FLASH_SECTOR_SIZE * count;
  printf("disk write %i %i\n", index, length);
  sleep_ms(100);
  flash_range_erase(FLASH_TARGET_OFFSET + index, length);
  sleep_ms(100);
  flash_range_program(FLASH_TARGET_OFFSET + index, buff, length);
  // memcpy(&FileSystem[index], &buff[0], length * sizeof(uint8_t));
  return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, /* [IN] Drive number */
                   BYTE cmd,  /* [IN] Control command code */
                   void* buff /* [I/O] Parameter and data buffer */
) {
  printf("disk_ioctl [%i]\n", cmd);
  uint8_t response;
  switch (cmd) {
    case GET_BLOCK_SIZE:
      *(DWORD*)buff = FLASH_BLOCK_SIZE;
      break;
    case GET_SECTOR_SIZE:
      *(DWORD*)buff = FLASH_SECTOR_SIZE;
      break;
    case GET_SECTOR_COUNT: {
      *(DWORD*)buff = 200;
      break;
    }
    default:
      break;
  }
  return RES_OK;
}

#endif
