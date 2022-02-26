#include "ff.h"
#include "diskio.h"

DSTATUS disk_status(BYTE pdrv /* [IN] Physical drive number */
);

DSTATUS disk_initialize(BYTE pdrv /* [IN] Physical drive number */
);

DRESULT disk_read(BYTE pdrv,    /* [IN] Physical drive number */
                  BYTE* buff,   /* [OUT] Pointer to the read data buffer */
                  LBA_t sector, /* [IN] Start sector number */
                  UINT count    /* [IN] Number of sectros to read */
);

DRESULT disk_write(
    BYTE pdrv,        /* [IN] Physical drive number */
    const BYTE* buff, /* [IN] Pointer to the data to be written */
    LBA_t sector,     /* [IN] Sector number to write from */
    UINT count        /* [IN] Number of sectors to write */
);

DRESULT disk_ioctl(BYTE pdrv, /* [IN] Drive number */
                   BYTE cmd,  /* [IN] Control command code */
                   void* buff /* [I/O] Parameter and data buffer */
);
