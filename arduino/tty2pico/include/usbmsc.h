#ifndef USBMSC_H
#define USBMSC_H

#include "Adafruit_TinyUSB.h"
#include "storage.h"

static Adafruit_USBD_MSC msc;
static bool mscReady;

/*******************************************************************************
 * Flash MSC Callbacks
 *******************************************************************************/

static int32_t mscFlashReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	int32_t result = flash.readBlocks(lba, (uint8_t*) buffer, bufsize / 512) ? bufsize : -1;
	return result;
}

static int32_t mscFlashWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	int32_t result = flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
	return result;
}

static void mscFlashFlushCallback(void)
{
	flash.syncBlocks(); // sync with flash
	flashfs.cacheClear(); // clear file system's cache to force refresh
	flashfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * SD MSC Callbacks
 *******************************************************************************/

static int32_t mscSDReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	int32_t result = sdfs.card()->readSectors(lba, (uint8_t *)buffer, bufsize / 512) ? bufsize : -1;
	return result;
}

static int32_t mscSDWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	int32_t result = sdfs.card()->writeSectors(lba, buffer, bufsize / 512) ? bufsize : -1;
	return result;
}

static void mscSDFlushCallback(void)
{
	sdfs.card()->syncDevice();
	sdfs.cacheClear(); // clear file system's cache to force refresh
	sdfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

bool getMscReady()
{
	return mscReady;
}

void setupUsbMsc()
{
	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	if (getHasSD())
		msc.setID("tty2pico", "SD Storage", "1.0");
	else
		msc.setID("tty2pico", "Flash Storage", "1.0");

	msc.begin();

	Serial.println("USB MSC setup complete");
}

void readyUsbMsc()
{
	if (mscReady)
		return;

	if (getHasSD())
	{
		msc.setReadWriteCallback(mscSDReadCallback, mscSDWriteCallback, mscSDFlushCallback);
		msc.setCapacity(sdfs.card()->sectorCount(), 512); // Set disk size, block size should be 512 regardless
		msc.setUnitReady(true); // MSC is ready for read/write
	}
	else
	{
		msc.setReadWriteCallback(mscFlashReadCallback, mscFlashWriteCallback, mscFlashFlushCallback);
		msc.setCapacity(flash.size() / 512, 512); // Set disk size, block size should be 512 regardless of spi flash page size
		msc.setUnitReady(true); // MSC is ready for read/write
	}

	mscReady = true;
	Serial.println("USB MSC ready");
}

// Lifecycle method for handling file system changes
void loopMSC()
{
	yield();

	if (flashfsChanged)
	{
		flashfsChanged = false;
	}

	if (sdfsChanged)
	{
		sdfsChanged = false;
	}
}

#endif
