#ifndef __NAND_FTL_DISKIO_H
#define __NAND_FTL_DISKIO_H

#include "pico/stdlib.h"
#include "stdlib.h"
#include "../fatfs/ff.h"      // BYTE type
#include "../fatfs/diskio.h"  // types from the diskio driver

DSTATUS nand_ftl_diskio_initialize(void);
DSTATUS nand_ftl_diskio_status(void);
DRESULT nand_ftl_diskio_read(BYTE *buff, LBA_t sector, UINT count);
DRESULT nand_ftl_diskio_write(const BYTE *buff, LBA_t sector, UINT count);
DRESULT nand_ftl_diskio_ioctl(BYTE cmd, void *buff);

#endif  // __NAND_FTL_DISKIO_H
