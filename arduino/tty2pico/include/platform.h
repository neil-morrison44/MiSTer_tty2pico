#ifndef TTY2PICO_PLATFORM_H
#define TTY2PICO_PLATFORM_H

#include "config.h"
#include "SpiDriver/SdSpiDriver.h"
#include "definitions.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <UnixTime.h>

static queue_t cmdQ;
static char datetimeString[37]; // Max date string could be 37 chars: Wednesday 17 November 01:59:49 2022

SpiPort_t *getSpiSD(void)
{
	SDCARD_SPI.setRX(SDCARD_MISO_PIN);
	SDCARD_SPI.setTX(SDCARD_MOSI_PIN);
	SDCARD_SPI.setSCK(SDCARD_SCK_PIN);

	return &SDCARD_SPI;
}

const char *getTime(int format)
{
	if (!rtc_running())
		return "RTC Disabled";

	datetime_t datetime;
	rtc_get_datetime(&datetime);
	memset(datetimeString, 0, sizeof(datetimeString));

	switch (format)
	{
		case DTF_HUMAN:
			datetime_to_str(datetimeString, sizeof(datetimeString), &datetime);
			return static_cast<const char *>(datetimeString);

		default:
			UnixTime unixTime(0);
			unixTime.setDateTime(datetime.year, datetime.month, datetime.day, datetime.hour, datetime.min, datetime.sec);
			String unixTimeString(unixTime.getUnix());
			memcpy(datetimeString, unixTimeString.c_str(), unixTimeString.length());
			return static_cast<const char *>(datetimeString);
	}
}

float getSpiRateDisplayMHz()
{
	spi_inst_t *spi = TFT_SPI_PORT == 0 ? spi0 : spi1;
	uint rate = spi_get_baudrate(spi);
	if (rate == UINT_MAX)
		rate = SPI_FREQUENCY;
	return rate / 1000000.0f;
}

float getSpiRateSdMHz()
{
	spi_inst_t *spi = &SDCARD_SPI == &SPI ? spi0 : spi1;
	uint rate = spi_get_baudrate(spi);
	return rate / 1000000.0f;
}

void setTime(uint32_t timestamp)
{
	if (!rtc_running())
		rtc_init();

	// Set the date and time
	UnixTime unixTime(0); // No timezone offset
	unixTime.getDateTime(timestamp);
	datetime_t datetime = {
		.year  = static_cast<int16_t>(unixTime.year),
		.month = static_cast<int8_t>(unixTime.month),
		.day   = static_cast<int8_t>(unixTime.day),
		.dotw  = static_cast<int8_t>(unixTime.dayOfWeek),
		.hour  = static_cast<int8_t>(unixTime.hour),
		.min   = static_cast<int8_t>(unixTime.minute),
		.sec   = static_cast<int8_t>(unixTime.second),
	};
	rtc_set_datetime(&datetime);
	delayMicroseconds(64); // Need to delay for 3 RTC clock cycles which is 64us

	// Validate RTC
	rtc_get_datetime(&datetime);
	memset(datetimeString, 0, sizeof(datetimeString));
	datetime_to_str(datetimeString, sizeof(datetimeString), &datetime);
	Serial.print("RTC date and time set to "); Serial.println(datetimeString);
}

void resetForUpdate(void)
{
	reset_usb_boot(0, 0);
}

void setupCPU(void)
{
	rp2040.enableDoubleResetBootloader();

	if (config.enableOverclock)
	{
		// Apply an overclock for 2x performance and a voltage tweak to stablize most RP2040 boards.
		// If it's good enough for pixel-pushing in MicroPython, it's good enough for us :P
		// https://github.com/micropython/micropython/issues/8208
		vreg_set_voltage(VREG_VOLTAGE_1_20); // Set voltage to 1.2v
		delay(10); // Allow vreg time to stabilize
		set_sys_clock_khz(266000, true); // Overclock to 266MHz
	}
}

void setupQueue(void)
{
	queue_init(&cmdQ, sizeof(CommandData), 1);
}

void addToQueue(CommandData &data)
{
	queue_try_add(&cmdQ, &data);
}

bool removeFromQueue(CommandData &data)
{
	return queue_try_remove(&cmdQ, &data);
}

#endif
