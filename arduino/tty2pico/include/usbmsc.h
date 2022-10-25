#ifndef USBMSC_H
#define USBMSC_H

#include "Adafruit_TinyUSB.h"
#include "platform.h"
#include "storage.h"
#include "display.h"

static Adafruit_USBD_MSC msc;

/*******************************************************************************
 * Flash MSC Callbacks
 *******************************************************************************/

static int32_t mscFlashReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	return flash.readSectors(lba, (uint8_t*) buffer, bufsize / FS_BLOCK_SIZE) ? bufsize : -1;
}

static int32_t mscFlashWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	return flash.writeSectors(lba, buffer, bufsize / FS_BLOCK_SIZE) ? bufsize : -1;
}

static void mscFlashFlushCallback(void)
{
	flash.syncDevice();
	flashfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * SD MSC Callbacks
 *******************************************************************************/

static int32_t mscSDReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	pauseBackground();
	int32_t result = sdfs.card()->readSectors(lba, (uint8_t *)buffer, bufsize / FS_BLOCK_SIZE) ? bufsize : -1;
	resumeBackground();
	return result;
}

static int32_t mscSDWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	pauseBackground();
	int32_t result = sdfs.card()->writeSectors(lba, buffer, bufsize / FS_BLOCK_SIZE) ? bufsize : -1;
	resumeBackground();
	return result;
}

static void mscSDFlushCallback(void)
{
	pauseBackground();
	sdfs.card()->syncDevice();
	resumeBackground();
	sdfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

void setupUsbMsc()
{
	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	if (getHasSD())
		msc.setID("tty2pico", "SD Storage", "1.0");
	else
		msc.setID("tty2pico", "Flash Storage", "1.0");

	if (getHasSD())
	{
		msc.setReadWriteCallback(mscSDReadCallback, mscSDWriteCallback, mscSDFlushCallback);
		msc.setCapacity(sdfs.card()->sectorCount(), FS_BLOCK_SIZE);
	}
	else
	{
		msc.setReadWriteCallback(mscFlashReadCallback, mscFlashWriteCallback, mscFlashFlushCallback);
		msc.setCapacity(flash.size() / FS_BLOCK_SIZE, FS_BLOCK_SIZE);
	}

	// MSC is ready for read/write
	msc.begin();
	msc.setUnitReady(true);
	Serial.println("USB MSC ready");
}

void beginUsbMsc()
{
	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	if (getHasSD())
		msc.setID("tty2pico", "SD Storage", "1.0");
	else
		msc.setID("tty2pico", "Flash Storage", "1.0");

	msc.begin();
	msc.setUnitReady(false);

	Serial.println("USB MSC started");
}

void readyUsbMsc()
{
	static bool mscReady;

	if (mscReady)
		return;

	if (getHasSD())
	{
		msc.setReadWriteCallback(mscSDReadCallback, mscSDWriteCallback, mscSDFlushCallback);
		msc.setCapacity(sdfs.card()->sectorCount(), FS_BLOCK_SIZE);
	}
	else
	{
		msc.setReadWriteCallback(mscFlashReadCallback, mscFlashWriteCallback, mscFlashFlushCallback);
		msc.setCapacity(flash.size() / FS_BLOCK_SIZE, FS_BLOCK_SIZE);
	}

	// MSC is ready for read/write
	msc.setUnitReady(true);
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
