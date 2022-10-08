#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"
#include <SPI.h>
#include <SdFat.h>
#include "Adafruit_SPIFlash.h"
#include <ff.h>
#include <diskio.h>
#include <AnimatedGIF.h>
#include <JPEGDEC.h>
#include <PNGdec.h>

// Un-comment to run with custom flash storage
// #define FLASHFS_CUSTOM_CS   A5
// #define FLASHFS_CUSTOM_SPI  SPI

#if defined(FLASHFS_CUSTOM_CS) && defined(FLASHFS_CUSTOM_SPI)
	Adafruit_FlashTransport_SPI flashTransport(FLASHFS_CUSTOM_CS, FLASHFS_CUSTOM_SPI);
#elif defined(ARDUINO_ARCH_ESP32)
	Adafruit_FlashTransport_ESP32 flashTransport;
#elif defined(ARDUINO_ARCH_RP2040)
	Adafruit_FlashTransport_RP2040 flashTransport((1 * 1024 * RESERVED_FLASH_KB), 0);
#else
	#if defined(EXTERNAL_FLASH_USE_QSPI)
		Adafruit_FlashTransport_QSPI flashTransport;
	#elif defined(EXTERNAL_FLASH_USE_SPI)
		Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
	#else
		#error No QSPI/SPI flash are defined on your board variant.h !
	#endif
#endif

Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem flashfs;
FATFS elmchamFatfs;
uint8_t workbuf[4096];
bool flashfsFormatted;
bool flashfsChanged;

static bool hasSD = false;
SdFat sdfs(&SDCARD_SPI);

/*************************
 * Helper functions
 *************************/

// Since SdFat doesn't fully support FAT12 such as format a new flash
// We will use Elm Cham's fatfs f_mkfs() to format
void formatFlash(void)
{
	// This is mostly copy/pasta of the example code for formatting using the FatFs lib:
	// https://github.com/adafruit/Adafruit_SPIFlash/blob/master/examples/SdFat_format/SdFat_format.ino

	// Call fatfs begin and passed flash object to initialize file system
	Serial.println("Creating FAT filesystem (this takes ~60 seconds)...");

	// Make filesystem.
	FRESULT r = f_mkfs("", FM_FAT | FM_SFD, 0, workbuf, sizeof(workbuf));
	if (r != FR_OK)
	{
		Serial.print("Error, f_mkfs failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	Serial.println("Filesystem created, attempting to mount");

	r = f_mount(&elmchamFatfs, "0:", 1);
	if (r != FR_OK)
	{
		Serial.print("Error, f_mount failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	Serial.println("Setting disk label to: " DISK_LABEL);

	r = f_setlabel(DISK_LABEL);
	if (r != FR_OK)
	{
		Serial.print("Error, f_setlabel failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	f_unmount("0:");

	flash.syncBlocks(); // sync to make sure all data is written to flash

	Serial.println("Formatted flash!");

	flashfsFormatted = flashfs.begin(&flash); // Try to mount one more time
	if (!flashfsFormatted)
	{
		Serial.println("Error, failed to mount newly formatted filesystem!");
		while(1) delay(1);
	}
}

File getFile(const char *path, oflag_t oflag = O_RDONLY)
{
	File file;

	if (hasSD)
	{
		// SD card
		if (!sdfs.exists(path))
		{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
			Serial.print("File not found: "); Serial.println(path);
#endif
			return file;
		}

		file.close(); // Ensure any previous file has been closed
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		if (file.open(&sdfs, path, oflag))
			Serial.print("Opened file: "); Serial.println(path);
#else
		file.open(&sdfs, path, oflag);
#endif
	}
	else
	{
		// Flash filesystem
		if (!flashfs.exists(path))
		{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
			Serial.print("File not found: "); Serial.println(path);
#endif
			return file;
		}

		file.close(); // Ensure any previous file has been closed
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		if (file.open(&flashfs, path, oflag))
			Serial.print("Opened file: "); Serial.println(path);
#else
		file.open(&flashfs, path, oflag);
#endif

		return file;
	}

	return file;
}

File getFile(String path, oflag_t oflag = O_RDONLY)
{
	return getFile(path.c_str(), oflag);
}

/*************************
 * Setup functions
 *************************/

static void setupFlash(void)
{
	Serial.println("Setting up flash storage");

	if (!flash.begin())
	{
		Serial.println("Error, failed to initialize flash chip!");
		while(1) yield();
	}

	// Init file system on the flash
	flashfsFormatted = flashfs.begin(&flash);
	if (!flashfsFormatted)
	{
		formatFlash();

		// Try to mount one more time
		flashfsFormatted = flashfs.begin(&flash);
		if (!flashfsFormatted)
		{
			// Couldn't set up filesystem fallback, can't really do anything except message the user
			Serial.println("Error, failed to mount newly formatted filesystem!");
			while(1) delay(1);
		}
	}

	Serial.println("TTY2PICO Flash Storage");
	Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
	Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");

	flashfsChanged = true;
}

static void setupSD(void)
{
	Serial.println("Setting up SD storage");

	SDCARD_SPI.setRX(SDCARD_MISO_PIN);
	SDCARD_SPI.setTX(SDCARD_MOSI_PIN);
	SDCARD_SPI.setSCK(SDCARD_SCK_PIN);

	if (!sdfs.begin(SDCARD_CS_PIN, SPI_FULL_SPEED))
	{
		Serial.println("SD initialization failed!");
		return;
	}

	hasSD = true;
	Serial.println("SD storage setup complete");
}

void setupStorage(void)
{
	setupFlash();
	setupSD();
}

/*************************
 * File/Directory functions
 *************************/

static File dir;
static String dirText;

bool fileExists(String path)
{
	if (hasSD)
		return sdfs.exists(path.c_str());
	else
		return flashfs.exists(path.c_str());
}

String getName(File *file)
{
	char filename[250];
	file->getName(filename, 250);
	return String(filename);
}

String getFullName(File *file, const char *directory = nullptr)
{
	if (directory == nullptr)
		return dirText + getName(file);
	else
		return String(directory) + getName(file);
}

String getDirectory(void)
{
	return getFullName(&dir);
}

int getFileCount(void)
{
	dir.rewindDirectory();

	int count = 0;
	while (true)
	{
		File entry = dir.openNextFile();
		if (entry)
			count += 1;
		else
			break;
	}

	dir.rewindDirectory();
	return count;
}

String getNextFile(void)
{
	static File entry;

	entry = dir.openNextFile();
	if (!entry)
		return "";

	return getFullName(&entry);
}

void printDirectory(const char *path, int numTabs)
{
	File dir = getFile(path);
	File entry;

	while (entry.openNext(&dir, O_RDONLY))
	{
		for (uint8_t i = 0; i < numTabs; i++)
			Serial.print('\t');

		String fileName = getFullName(&entry, path);
		Serial.print(fileName);
		if (entry.isDir())
		{
			Serial.println("/");
			printDirectory(fileName.c_str(), numTabs + 1);
		}
		else
		{
			Serial.print("\t\t");
			entry.printFileSize(&Serial);
			Serial.print("\tCREATION: ");
			entry.printCreateDateTime(&Serial);
			Serial.print("\tLAST WRITE: ");
			entry.printModifyDateTime(&Serial);
			Serial.println();
		}

		entry.close();
	}
}

inline int readFile(File *file, uint8_t *buffer, int32_t length, const char *errorMessage = nullptr)
{
	if (!file->available())
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.println(errorMessage);
#endif
		return 0;
	}

	int byteCount = file->read(buffer, length);
	return byteCount;
}

void rewindDirectory(void)
{
	dir.rewindDirectory();
}

void setDirectory(String path)
{
	dir.close();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Setting directory to: "); Serial.println(path.c_str());
#endif
	dirText = path;
	dir = getFile(path);
}

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
	int32_t byteCount = readFile(file, buffer, length, "Couldn't read GIF file");
	page->iPos = file->position();
	return byteCount;
}

int32_t gifSeek(GIFFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
	file->seek(position);
	page->iPos = file->position();
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
	return readFile(file, buffer, length, "Couldn't read JPEG file");
}

int32_t jpegSeek(JPEGFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
	file->seek(position);
	page->iPos = file->position();
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
	return readFile(file, buffer, length, "Couldn't read PNG file");
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
	File *file = static_cast<File *>(page->fHandle);
	file->seek(position);
	page->iPos = file->position();
	return page->iPos;
}

/*************************
 * FatFs implementation
 *************************/

DSTATUS disk_status ( BYTE pdrv )
{
	(void) pdrv;
	return 0;
}

DSTATUS disk_initialize ( BYTE pdrv )
{
	(void) pdrv;
	return 0;
}

DRESULT disk_read (
	BYTE pdrv,    /* Physical drive nmuber to identify the drive */
	BYTE *buff,   /* Data buffer to store read data */
	DWORD sector, /* Start sector in LBA */
	UINT count    /* Number of sectors to read */
)
{
	(void) pdrv;
	return flash.readBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write (
	BYTE pdrv,        /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	DWORD sector,     /* Start sector in LBA */
	UINT count        /* Number of sectors to write */
)
{
	(void) pdrv;
	return flash.writeBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl (
	BYTE pdrv,  /* Physical drive nmuber (0..) */
	BYTE cmd,   /* Control code */
	void *buff  /* Buffer to send/receive control data */
)
{
	(void) pdrv;

	switch ( cmd )
	{
		case CTRL_SYNC:
			flash.syncBlocks();
			return RES_OK;

		case GET_SECTOR_COUNT:
			*((DWORD*) buff) = flash.size() / 512;
			return RES_OK;

		case GET_SECTOR_SIZE:
			*((WORD*) buff) = 512;
			return RES_OK;

		case GET_BLOCK_SIZE:
			*((DWORD*) buff) = 8; // erase block size in units of sector size
			return RES_OK;

		default:
			return RES_PARERR;
	}
}

#endif
