#ifndef STORAGE_H
#define STORAGE_H

#include <AnimatedGIF.h>
#include <PNGdec.h>

/*************************
 * Setup functions
 *************************/

extern void setupStorage(void);

/*************************
 * Directory functions
 *************************/

extern String getDirectory(void);
extern int getFileCount(void);
extern String getNextFile(void);
extern void printDirectory(const char *path, int numTabs);
extern void rewindDirectory(void);
extern void setDirectory(String path);

/*************************
 * GIF functions
 *************************/

extern void *gifOpen(const char *filename, int32_t *size);
extern void gifClose(void *handle);
extern int32_t gifRead(GIFFILE *page, uint8_t *buffer, int32_t length);
extern int32_t gifSeek(GIFFILE *page, int32_t position);

/*************************
 * PNG functions
 *************************/

extern void *pngOpen(const char *filename, int32_t *size);
extern void pngClose(void *handle);
extern int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length);
extern int32_t pngSeek(PNGFILE *page, int32_t position);

/*************************
 * Include implementation
 *************************/

#if defined(STORAGE_TYPE_SD_CARD)
#include "storage/sdcard.h"
#else
#include "storage/msc.h"
#endif

#endif
