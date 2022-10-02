#ifndef MSC_H
#define MSC_H

#define ENABLE_EXTENDED_TRANSFER_CLASS 1
#define FAT12_SUPPORT 1

#include "../config.h"
#include <SPI.h>
#include <PNGdec.h>
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"

// Un-comment to run with custom flash storage
// #define CUSTOM_CS   A5
// #define CUSTOM_SPI  SPI

#if defined(CUSTOM_CS) && defined(CUSTOM_SPI)
	Adafruit_FlashTransport_SPI flashTransport(CUSTOM_CS, CUSTOM_SPI);
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
	// sync with flash
	flash.syncBlocks();

	// clear file system's cache to force refresh
	fatfs.cacheClear();

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

FatFile getFile(const char *path, oflag_t oflag = O_RDONLY)
{
  FatFile file;

	if (!fatfs.exists(path))
		return file;

  file.close(); // Ensure any previous file has been closed
	file.open(&fatfs, path, oflag);
  return file;
}

FatFile getFile(String path, oflag_t oflag = O_RDONLY)
{
  return getFile(path.c_str(), oflag);
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

  // Delay just a moment to allow filesystem to initialize
  delay(500);

	if (!fs_formatted)
		Serial.println("Failed to init files system, flash may not be formatted");

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
 * PNG functions
 *************************/

static FatFile pngfile;

void *pngOpen(const char *filename, int32_t *size)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
  Serial.printf("Attempting to open %s\n", filename);
#endif
  pngfile = getFile(filename);
  *size = pngfile.fileSize();
  return &pngfile;
}

void pngClose(void *handle)
{
  if (handle == nullptr)
    return;
    
  FatFile pngfile = *((FatFile*)handle);
  pngfile.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length)
{
  if (!pngfile.isOpen())
    return 0;
    
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
  if (!pngfile.isOpen())
    return 0;

  page = page; // Avoid warning
  return pngfile.seekSet(position);
}

#endif
