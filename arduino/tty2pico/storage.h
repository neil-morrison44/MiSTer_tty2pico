#ifndef STORAGE_H
#define STORAGE_H

#define ENABLE_EXTENDED_TRANSFER_CLASS 1
#define FAT12_SUPPORT 1

#include "config.h"
#include <SPI.h>
#if defined(STORAGE_TYPE_SD_CARD)
#include <SD.h>
#else
#include <SdFat.h>
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <ff.h>
#include <diskio.h>
#endif
#include <AnimatedGIF.h>
#include <JPEGDEC.h>
#include <PNGdec.h>

/*************************
 * Helper functions
 *************************/

extern File getFile(const char *path, oflag_t oflag);
File getFile(String path, oflag_t oflag) { return getFile(path.c_str(), oflag); }
File getFile(const char *path) { return getFile(path, O_RDONLY); }
File getFile(String path) { return getFile(path, O_RDONLY); }

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

static File giffile;

void *gifOpen(const char *filename, int32_t *size)
{
	giffile = getFile(filename);

	if (giffile.available())
	{
		*size = giffile.size();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened file "); Serial.print(String(filename).c_str()); Serial.print(" with file size "); Serial.print(giffile.size()); Serial.println(" bytes");
#endif
		return (void *)&giffile;
	}
	else
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Failed to open "); Serial.println(String(filename).c_str());
#endif
		return NULL;
	}
}

void gifClose(void *handle)
{
	if (handle == nullptr)
		return;

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Closing file");
#endif
	File *file = static_cast<File *>(handle);
	file->close();
}

int32_t gifRead(GIFFILE *page, uint8_t *buffer, int32_t length)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Reading GIF");
#endif
	if (!file->available())
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.println("GIF not open");
#endif
		return 0;
	}

	int32_t byteCount;
	byteCount = length;
	// Note: If you read a file all the way to the last byte, seek() stops working
	if ((page->iSize - page->iPos) < length)
			byteCount = page->iSize - page->iPos - 1; // <-- ugly work-around
	if (byteCount <= 0)
			return 0;
	byteCount = file->read(buffer, byteCount);
	page->iPos = file->position();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Read "); Serial.print(byteCount); Serial.println(" bytes");
#endif
	return byteCount;
}

int32_t gifSeek(GIFFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeking GIF to position "); Serial.println(position);
#endif
	file->seek(position);
	page->iPos = (int32_t)file->position();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeked to position "); Serial.println(page->iPos);
#endif
	return page->iPos;
}

/*************************
 * JPEG functions
 *************************/

static File jpegfile;

void *jpegOpen(const char *filename, int32_t *size)
{
	jpegfile = getFile(filename);

	if (jpegfile.available())
	{
		*size = jpegfile.size();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened file "); Serial.print(String(filename).c_str()); Serial.print(" with file size "); Serial.print(jpegfile.size()); Serial.println(" bytes");
#endif
		return (void *)&jpegfile;
	}
	else
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Failed to open "); Serial.println(String(filename).c_str());
#endif
		return NULL;
	}
}

void jpegClose(void *handle)
{
	if (handle == nullptr)
		return;

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Closing file");
#endif
	File *file = static_cast<File *>(handle);
	file->close();
}

int32_t jpegRead(JPEGFILE *page, uint8_t *buffer, int32_t length)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Reading JPEG");
#endif
	if (!file->available())
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.println("JPEG not open");
#endif
		return 0;
	}

	int byteCount = file->read(buffer, length);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Read "); Serial.print(byteCount); Serial.println(" bytes");
#endif
	return byteCount;
}

int32_t jpegSeek(JPEGFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeking JPEG to position "); Serial.println(position);
#endif
	file->seek(position);
	page->iPos = (int32_t)file->position();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeked to position "); Serial.println(page->iPos);
#endif
	return page->iPos;
}

/*************************
 * PNG functions
 *************************/

static File pngfile;

void *pngOpen(const char *filename, int32_t *size)
{
	pngfile = getFile(filename);

	if (pngfile.available())
	{
		*size = pngfile.size();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened file "); Serial.print(String(filename).c_str()); Serial.print(" with file size "); Serial.print(pngfile.size()); Serial.println(" bytes");
#endif
		return &pngfile;
	}
	else
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Failed to open "); Serial.println(String(filename).c_str());
#endif
		return NULL;
	}
}

void pngClose(void *handle)
{
	if (handle == nullptr)
		return;

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Closing file");
#endif
	File *file = static_cast<File *>(handle);
	file->close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Reading PNG");
#endif
	if (!file->available())
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.println("PNG not open");
#endif
		return 0;
	}

	int byteCount = file->read(buffer, length);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Read "); Serial.print(byteCount); Serial.println(" bytes");
#endif
	return byteCount;
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeking PNG to position "); Serial.println(position);
#endif
	file->seek(position);
	page->iPos = (int32_t)file->position();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Seeked to position "); Serial.println(page->iPos);
#endif
	return page->iPos;
}

#if defined(STORAGE_TYPE_SD_CARD)
#include "storage/sdcard.h"
#else
#include "storage/msc.h"
#endif

#endif
