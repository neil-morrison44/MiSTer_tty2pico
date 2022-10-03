#ifndef MSC_H
#define MSC_H

#define ENABLE_EXTENDED_TRANSFER_CLASS 1
#define FAT12_SUPPORT 1

#include "../config.h"
#include <SPI.h>
#include <ff.h>
#include <diskio.h>
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"

// Un-comment to run with custom flash storage
// #define FLASHFS_CUSTOM_CS   A5
// #define FLASHFS_CUSTOM_SPI  SPI

#if defined(FLASHFS_CUSTOM_CS) && defined(FLASHFS_CUSTOM_SPI)
	Adafruit_FlashTransport_SPI flashTransport(FLASHFS_CUSTOM_CS, FLASHFS_CUSTOM_SPI);
#elif defined(ARDUINO_ARCH_ESP32)
	Adafruit_FlashTransport_ESP32 flashTransport;
#elif defined(ARDUINO_ARCH_RP2040)
	Adafruit_FlashTransport_RP2040 flashTransport;
#else
	#if defined(EXTERNAL_FLASH_USE_QSPI)
		Adafruit_FlashTransport_QSPI flashTransport;
	#elif defined(EXTERNAL_FLASH_USE_SPI)
		Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
	#else
		#error No QSPI/SPI flash are defined on your board variant.h !
	#endif
#endif

/*************************
 * USB MSC
 *************************/

Adafruit_USBD_MSC usb_msc;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
bool fs_formatted; // Check if flash is formatted
bool fs_changed;   // Set to true when PC write to flash

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t mscReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	// Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
	// already include 4K sector caching internally. We don't need to cache it, yahhhh!!
	return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t mscWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);

	// Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
	// already include 4K sector caching internally. We don't need to cache it, yahhhh!!
	return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void mscFlushCallback(void)
{
	flash.syncBlocks(); // sync with flash
	fatfs.cacheClear(); // clear file system's cache to force refresh
	fs_changed = true;
	digitalWrite(LED_BUILTIN, LOW);
}

void loopMSC()
{
	if (fs_changed)
	{
		fs_changed = false;

		// check if host formatted disk
		if (!fs_formatted)
			fs_formatted = fatfs.begin(&flash);

		// skip if still not formatted
		if (!fs_formatted)
			return;
	}
}

/*************************
 * Helper functions
 *************************/

void formatFlash(void)
{
	// This is mostly copy/pasta of the example code for formatting using the FatFs lib:
	// https://github.com/adafruit/Adafruit_SPIFlash/blob/master/examples/SdFat_format/SdFat_format.ino

	FATFS elmchamFatfs;
	uint8_t fs_workbuf[4096]; // Working buffer for f_fdisk function.

	// Make filesystem.
	FRESULT r = f_mkfs("", FM_FAT | FM_SFD, 0, fs_workbuf, sizeof(fs_workbuf));
	if (r != FR_OK)
	{
		Serial.print("Error, f_mkfs failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	// mount to set disk label
	r = f_mount(&elmchamFatfs, "0:", 1);
	if (r != FR_OK)
	{
		Serial.print("Error, f_mount failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	// Setting label
	Serial.println("Setting disk label to: " DISK_LABEL);
	r = f_setlabel(DISK_LABEL);
	if (r != FR_OK)
	{
		Serial.print("Error, f_setlabel failed with error code: "); Serial.println(r, DEC);
		while(1) yield();
	}

	f_mount(NULL, "0:", 1);

	// sync to make sure all data is written to flash
	flash.syncBlocks();

	Serial.println("Formatted flash!");

	// Try to mount one more time
	fs_formatted = fatfs.begin(&flash);
	if (!fs_formatted)
	{
		Serial.println("Error, failed to mount newly formatted filesystem!");
		while(1) delay(1);
	}
}

File getFile(const char *path, oflag_t oflag)
{
	File file;

	if (!fatfs.exists(path))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("File not found: "); Serial.println(path);
#endif
		return file;
	}

	file.close(); // Ensure any previous file has been closed
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	if (file.open(&fatfs, path, oflag))
		Serial.print("Opened file: "); Serial.println(path);
#else
	file.open(&fatfs, path, oflag);
#endif

	return file;
}

/*************************
 * Setup functions
 *************************/

void setupStorage(void)
{
	flash.begin();

	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	usb_msc.setID("TTY2PICO", "Flash Storage", "1.0");

	// Set callback
	usb_msc.setReadWriteCallback(mscReadCallback, mscWriteCallback, mscFlushCallback);

	// Set disk size, block size should be 512 regardless of spi flash page size
	usb_msc.setCapacity(flash.size() / 512, 512);

	// MSC is ready for read/write
	usb_msc.setUnitReady(true);

	usb_msc.begin();

	// Init file system on the flash
	fs_formatted = fatfs.begin(&flash);
	if (!fs_formatted)
	{
		formatFlash();

		// Try to mount one more time
		fs_formatted = fatfs.begin(&flash);
		if (!fs_formatted)
		{
			Serial.println("Error, failed to mount newly formatted filesystem!");
			while(1) delay(1);
		}
	}

	Serial.println("TTY2PICO Flash Storage");
	Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
	Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");

	fs_changed = true; // to print contents initially

	Serial.println("USB MSC storage setup complete");
}

/*************************
 * Directory functions
 *************************/

static FatFile dir;
static String dirText;

String getDirectory(void)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Get directory: "); Serial.println(dirText.c_str());
#endif
	return dirText;
}

int getFileCount(void)
{
	int count = 0;
	if (!dir.isDir() || !dir.available())
		return count;

	dir.rewind();
	FatFile entry;
	while (entry.openNext(&dir, O_RDONLY))
		count += 1;

	dir.rewind();
	return count;
}

String getNextFile(void)
{
	FatFile entry;

	if (entry.openNext(&dir, O_RDONLY))
	{
		char buffer[250];
		memset(buffer, 0, sizeof(char) * 250);
		entry.getName(buffer, 250);
		String fileName = dirText + String(buffer);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Got next file: "); Serial.println(fileName);
#endif
		return fileName;
	}
	else
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.println("No next file found");
#endif
		return "";
	}
}

void printDirectory(const char *path, int numTabs)
{
	FatFile dir = getFile(path);
	FatFile entry;

	while (entry.openNext(&dir, O_RDONLY))
	{
		for (uint8_t i = 0; i < numTabs; i++)
			Serial.print('\t');

		char buffer[250];
		memset(buffer, 0, sizeof(char) * 250);
		entry.getName(buffer, 250);
		String fileName = String(path) + String(buffer);
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

void rewindDirectory(void)
{
	dir.rewind();
}

void setDirectory(String path)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Setting directory to: "); Serial.println(dirText);
#endif
	dirText = path;

	if (dir.isDir())
		dir.close();

	dir = getFile(dirText);

	if (dir.isDir())
		dir.rewind();
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
