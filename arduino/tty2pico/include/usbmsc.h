#ifndef USBMSC_H
#define USBMSC_H

#include "Adafruit_TinyUSB.h"
#include "storage.h"

Adafruit_USBD_MSC msc;

/*******************************************************************************
 * Flash MSC Callbacks
 *******************************************************************************/

int32_t mscFlashReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	return flash.readBlocks(lba, (uint8_t*) buffer, bufsize / 512) ? bufsize : -1;
}

int32_t mscFlashWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

void mscFlashFlushCallback(void)
{
	flash.syncBlocks(); // sync with flash
	flashfs.cacheClear(); // clear file system's cache to force refresh
	flashfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * SD MSC Callbacks
 *******************************************************************************/

int32_t mscSDReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	return sdfs.card()->readBlocks(lba, (uint8_t *)buffer, bufsize / 512) ? bufsize : -1;
}

int32_t mscSDWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);
	return sdfs.card()->writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

void mscSDFlushCallback(void)
{
	sdfs.card()->syncBlocks();
	sdfs.cacheClear(); // clear file system's cache to force refresh
	sdfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

void setupUsbMsc()
{
	if (getHasSD())
	{
		// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
		msc.setID("tty2pico", "SD Storage", "1.0");
		msc.begin();
		msc.setReadWriteCallback(mscSDReadCallback, mscSDWriteCallback, mscSDFlushCallback);
		msc.setCapacity(sdfs.card()->cardSize(), 512); // Set disk size, block size should be 512 regardless
		msc.setUnitReady(true); // MSC is ready for read/write
	}
	else
	{
		// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
		msc.setID("tty2pico", "Flash Storage", "1.0");
		msc.begin();
		msc.setReadWriteCallback(mscFlashReadCallback, mscFlashWriteCallback, mscFlashFlushCallback);
		msc.setCapacity(flash.size() / 512, 512); // Set disk size, block size should be 512 regardless of spi flash page size
		msc.setUnitReady(true); // MSC is ready for read/write
	}

	Serial.println("USB MSC storage setup complete");
}

// Lifecycle method for handle file system changes
void loopMSC()
{
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
