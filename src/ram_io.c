#include "ff.h"
#include "stdio.h"
#include "string.h"
#include "diskio.h"

#ifdef IO_RAM

//#define BYTES_COUNT 1024 * 1024 * 4
#define BYTES_COUNT 512 * 200
#define SECTOR_SIZE 512
static uint8_t FileSystem[BYTES_COUNT];

DSTATUS disk_status(BYTE pdrv /* [IN] Physical drive number */
) {
  printf("disk_status \n");
  return 0;
}

DSTATUS disk_initialize(BYTE pdrv /* [IN] Physical drive number */
) {
  printf("disk_initialize \n");
  return 0;
};

DRESULT disk_read(BYTE pdrv,    /* [IN] Physical drive number */
                  BYTE* buff,   /* [OUT] Pointer to the read data buffer */
                  LBA_t sector, /* [IN] Start sector number */
                  UINT count    /* [IN] Number of sectros to read */
) {
  printf("disk read \n");
  int index = FF_MIN_SS * sector;
  int length = FF_MIN_SS * count;

  memcpy(&buff[0], &FileSystem[index], length * sizeof(uint8_t));
  return RES_OK;
};

DRESULT disk_write(
    BYTE pdrv,        /* [IN] Physical drive number */
    const BYTE* buff, /* [IN] Pointer to the data to be written */
    LBA_t sector,     /* [IN] Sector number to write from */
    UINT count        /* [IN] Number of sectors to write */
) {
  printf("disk write \n");
  int index = FF_MIN_SS * sector;
  int length = FF_MIN_SS * count;
  memcpy(&FileSystem[index], &buff[0], length * sizeof(uint8_t));
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
      *(DWORD*)buff = 1;
      break;
    case GET_SECTOR_COUNT: {
      *(DWORD*)buff = (BYTES_COUNT) / SECTOR_SIZE;
      break;
    }
    default:
      break;
  }
  return RES_OK;
}

#endif
