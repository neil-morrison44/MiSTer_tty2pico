#ifndef USBMSC_H
#define USBMSC_H

#include "Adafruit_TinyUSB.h"
#include "storage.h"

Adafruit_USBD_MSC usb_msc;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes (must be multiple of block size)
int32_t mscReadCallback(uint32_t lba, void* buffer, uint32_t bufsize)
{
	// Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks already include 4K sector caching internally. We don't need to cache it, yahhhh!!
	return flash.readBlocks(lba, (uint8_t*) buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes (must be multiple of block size)
int32_t mscWriteCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
	digitalWrite(LED_BUILTIN, HIGH);

	// Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks already include 4K sector caching internally. We don't need to cache it, yahhhh!!
	return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void mscFlushCallback(void)
{
	flash.syncBlocks(); // sync with flash
	flashfs.cacheClear(); // clear file system's cache to force refresh
	flashfsChanged = true;
	digitalWrite(LED_BUILTIN, LOW);
}

void setupUsbMsc()
{
	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	usb_msc.setID("TTY2PICO", "Flash Storage", "1.0");

	// Set callback
	usb_msc.setReadWriteCallback(mscReadCallback, mscWriteCallback, mscFlushCallback);

	// Set disk size, block size should be 512 regardless of spi flash page size
	usb_msc.setCapacity(flash.size() / 512, 512);

	// MSC is ready for read/write
	usb_msc.setUnitReady(true);

	usb_msc.begin();

	Serial.println("USB MSC storage setup complete");
}

void loopMSC()
{
	if (flashfsChanged)
	{
		flashfsChanged = false;

		// check if host formatted disk
		if (!flashfsFormatted)
			flashfsFormatted = flashfs.begin(&flash);

		// skip if still not formatted
		if (!flashfsFormatted)
			return;
	}
}

#endif
