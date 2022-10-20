#ifndef STORAGE_H
#define STORAGE_H

#define FS_BLOCK_SIZE 512 // Always use 512k block size

// Makes PlatformIO intellisense happy
#ifdef SD_FAT_TYPE
#undef SD_FAT_TYPE
#define SD_FAT_TYPE 3
#endif

#include "config.h"
#include "platform.h"
#include <SPI.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include <ff.h>
#include <diskio.h>
#include <AnimatedGIF.h>
#include <PNGdec.h>
#include "mutex"

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
FsVolume flashfs;
bool flashfsFormatted = false;
bool flashfsChanged = false;

SdFs sdfs;
static bool hasSD = false;
bool sdfsChanged;

FsVolume *volume = nullptr; // Pointer to the active volume

uint8_t workbuf[4096];
FATFS fatfs;

/*************************
 * Helper functions
 *************************/

// Since SdFat won't format anything smaller than 6MB use Elm Cham's fatfs f_mkfs() to format
void formatFlash(void)
{
	// This is mostly copy/pasta of the example code for formatting using the FatFs lib:
	// https://github.com/adafruit/Adafruit_SPIFlash/blob/master/examples/SdFat_format/SdFat_format.ino

	// Call fatfs begin and passed flash object to initialize filesystem
	Serial.println("Creating FAT filesystem (this takes ~60 seconds)...");

	// Make filesystem.
	FRESULT r = f_mkfs("", FM_FAT | FM_SFD, 0, workbuf, sizeof(workbuf));
	if (r != FR_OK)
	{
		Serial.print("Error, f_mkfs failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	Serial.println("Filesystem created, attempting to mount");

	r = f_mount(&fatfs, "0:", 1);
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

	flash.syncDevice(); // sync to make sure all data is written to flash

	Serial.println("Formatted flash!");

	flashfsFormatted = flashfs.begin(&flash); // Try to mount one more time
	if (!flashfsFormatted)
	{
		Serial.println("Error, failed to mount newly formatted filesystem! You may need to format the flash volume manually.");
	}
}

FsFile getFile(const char *path, oflag_t oflag = O_RDONLY)
{
	FsFile file;

#if VERBOSE_OUTPUT == 1
	Serial.print("Looking for file: "); Serial.println(path);
#endif
	if (volume->exists(path))
	{
#if VERBOSE_OUTPUT == 1
	Serial.print("Attempting to open file: "); Serial.println(path);
#endif
		file = volume->open(path, oflag);
#if VERBOSE_OUTPUT == 1
		if (file)
		{
			Serial.print("Opened file: "); Serial.println(path);
		}
		else
		{
			Serial.print("Couldn't open file: "); Serial.print(path); Serial.print(", error code: "); Serial.println(file.getError());
		}
	}
	else
	{
		Serial.print("File not found: "); Serial.print(path); Serial.print(", error code: "); Serial.println(file.getError());
	}
#else
	}
#endif

	return file;
}

FsFile getFile(String path, oflag_t oflag = O_RDONLY)
{
	return getFile(path.c_str(), oflag);
}

bool getHasSD(void)
{
	return hasSD;
}

void saveFile(String path, const char *data, int size, oflag_t oflag = (O_WRITE | O_CREAT | O_TRUNC))
{
	FsFile file = getFile(path, oflag);
	if (!file)
	{
		Serial.print("Unable to save file: "); Serial.println(path);
		return;
	}

	if (file.write(data, size))
	{
#if VERBOSE_OUTPUT == 1
		Serial.print("Saved file "); Serial.println(path);
#endif
	}
	else
	{
		String message = "Failed to save file " + path;
		Serial.println(message.c_str());
	}
	file.close();
}

/*************************
 * Setup functions
 *************************/

static void loadConfig(void)
{
	Serial.println("Trying to load config...");
	FsFile configFile = getFile(CONFIG_FILE_PATH);
	if (configFile)
	{
		// Read entire file into memory, should only be a few KB max
		char *buffer = (char *)malloc(sizeof(char) * configFile.size());
		Serial.println("Read config start");
		configFile.read(buffer, configFile.size());
		configFile.close();
		Serial.println("Read config complete");
		const char *error = parseConfig(buffer);
		free(buffer);

		// Couldn't parse config
		if (error)
			Serial.println(error);
		else
			Serial.println("Config file loaded");
	}
	else Serial.println("No config file found");
}

void saveConfig(void)
{
	Serial.println("Config file saving temporarily disabled");
	// int bufferSize = 4096;
	// char buffer[bufferSize]; // Allocate 4K buffer to handle config data
	// int size = exportConfig(buffer, bufferSize);
	// saveFile(CONFIG_FILE_PATH, buffer, size);
	// Serial.println("Config file saved to " + String(CONFIG_FILE_PATH));
}

static void setupFlash(void)
{
	Serial.println("Setting up flash storage");

	if (!flash.begin())
	{
		Serial.println("Error, failed to initialize flash chip!");
		while(1) yield();
	}

	Serial.println("TTY2PICO Flash Storage");
	Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
	Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");

	// Init filesystem on the flash
	flashfsFormatted = flashfs.begin(&flash);
	if (!flashfsFormatted)
	{
		// Couldn't init filesystem, automatically format to ensure it's working
		formatFlash();

		// Try to mount one more time
		flashfsFormatted = flashfs.begin(&flash);
		if (!flashfsFormatted)
		{
			// Couldn't set up filesystem fallback, can't really do anything except message the user
			Serial.println("Error, failed to mount filesystem! You may need to format it manually as a FAT32 partition.");
		}
	}
}

static void setupSD(void)
{
	Serial.println("Setting up SD storage");

	SdSpiConfig spiConfig = getSdSpiConfig();

	if (!sdfs.begin(spiConfig))
	{
		sdfs.errorPrint(&Serial);
		return;
	}

	FsFile root = sdfs.open("/", O_RDONLY);
	if (root)
	{
		if (root.isDir())
		{
			hasSD = root.isDir();
			Serial.println("SD filesystem initialized");
		}
		else
		{
			Serial.println("SD filesystem intialization failed, unable to locate root filesystem");
		}
		root.close();
	}
	else
	{
		Serial.print("SD filesystem intialization failed, error code: "); Serial.println(root.getError());
	}
}

void setupStorage(void)
{
#ifdef SDCARD_SPI
	setupSD();
#endif
	if (!hasSD)
		setupFlash();

	if (flashfsFormatted || hasSD)
	{
		volume = hasSD ? sdfs.vol() : &flashfs;
		loadConfig();
	}
}

/*************************
 * File/Directory functions
 *************************/

static String dirText;
static FsFile dir;

bool fileExists(String path)
{
	return volume->exists(path.c_str());
}

String getName(FsFile *file)
{
	char filename[250];
	file->getName(filename, 250);
	return String(filename);
}

String getFullName(FsFile *file, const char *directory = nullptr)
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
	if (!dir)
		return -1;

	dir.rewindDirectory();
	int count = 0;
	while (true)
	{
		if (dir.openNextFile())
			count += 1;
		else
			break;
	}
	dir.rewindDirectory();

	return count;
}

String getNextFile(void)
{
	FsFile entry = dir.openNextFile();
	return (entry) ? getFullName(&entry) : "";
}

void printDirectory(const char *path, int numTabs)
{
	FsFile dir = getFile(path);
	FsFile entry;

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

inline int readFile(FsFile *file, uint8_t *buffer, int32_t length, const char *errorMessage = nullptr)
{
	if (file->available())
		return file->read(buffer, length);

#if VERBOSE_OUTPUT == 1
	Serial.println(errorMessage);
#endif
	return 0;
}

void rewindDirectory(void)
{
	if (dir)
		dir.rewindDirectory();
}

void setDirectory(String path)
{
	if (dir)
		dir.close();

	dir = getFile(path);
	if (dir)
	{
		dirText = path;
#if VERBOSE_OUTPUT == 1
		Serial.print("Directory set to: "); Serial.println(path.c_str());
#endif
	}
#if VERBOSE_OUTPUT == 1
	else
	{
		Serial.print("Couldn't set directory to: "); Serial.print(path.c_str()); Serial.print(", error code "); Serial.println(dir.getError());
	}
#endif
}

/*************************
 * GIF functions
 *************************/

void *gifOpen(const char *filename, int32_t *size)
{
	static FsFile giffile;

	giffile = getFile(filename);

	if (giffile.available())
	{
		*size = giffile.size();
#if VERBOSE_OUTPUT == 1
		Serial.print("Opened file "); Serial.print(String(filename).c_str()); Serial.print(" with file size "); Serial.print(giffile.size()); Serial.println(" bytes");
#endif
		return &giffile;
	}

#if VERBOSE_OUTPUT == 1
	Serial.print("Failed to open "); Serial.println(String(filename).c_str());
#endif
	return NULL;
}

void gifClose(void *handle)
{
	if (handle == nullptr)
		return;

#if VERBOSE_OUTPUT == 1
	Serial.println("Closing file");
#endif
	static_cast<FsFile *>(handle)->close();
}

int32_t gifRead(GIFFILE *page, uint8_t *buffer, int32_t length)
{
	FsFile *file = static_cast<FsFile *>(page->fHandle);
	int byteCount = readFile(file, buffer, length, "Couldn't read GIF file");
	page->iPos = file->position();
	return byteCount;
}

int32_t gifSeek(GIFFILE *page, int32_t position)
{
	FsFile *file = static_cast<FsFile *>(page->fHandle);
	file->seek(position);
	page->iPos = file->position();
	return page->iPos;
}

/*************************
 * PNG functions
 *************************/

void *pngOpen(const char *filename, int32_t *size)
{
	static FsFile pngfile;

	pngfile = getFile(filename);

	if (pngfile.available())
	{
		*size = pngfile.size();
#if VERBOSE_OUTPUT == 1
		Serial.print("Opened file "); Serial.print(String(filename).c_str()); Serial.print(" with file size "); Serial.print(pngfile.size()); Serial.println(" bytes");
#endif
		return &pngfile;
	}
	else
	{
#if VERBOSE_OUTPUT == 1
		Serial.print("Failed to open "); Serial.println(String(filename).c_str());
#endif
		return NULL;
	}
}

void pngClose(void *handle)
{
	if (handle == nullptr)
		return;

#if VERBOSE_OUTPUT == 1
	Serial.println("Closing file");
#endif
	static_cast<FsFile *>(handle)->close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length)
{
	int byteCount = readFile(static_cast<FsFile *>(page->fHandle), buffer, length, "Couldn't read PNG file");
	return byteCount;
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
	FsFile *file = static_cast<FsFile *>(page->fHandle);
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
	return flash.readSectors(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write (
	BYTE pdrv,        /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	DWORD sector,     /* Start sector in LBA */
	UINT count        /* Number of sectors to write */
)
{
	(void) pdrv;
	return flash.writeSectors(sector, buff, count) ? RES_OK : RES_ERROR;
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
			flash.syncDevice();
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
