#ifndef TTY2PICO_PLATFORM_IMPL_H
#define TTY2PICO_PLATFORM_IMPL_H

#include "config.h"
#include "definitions.h"
#include "SPI.h"
#include "api/HardwareSPI.h"
#include "SpiDriver/SdSpiDriver.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <UnixTime.h>

static SdSpiDriverT2P sdSpiDriver;

static queue_t cmdQ;
static char datetimeString[37]; // Max date string could be 37 chars: Wednesday 17 November 01:59:49 2022

/*******************************************************************************
 * Helper functions
 *******************************************************************************/

inline spi_cpol_t get_cpol(SPIMode mode)
{
	switch (mode)
	{
		case SPI_MODE0:
			return SPI_CPOL_0;
		case SPI_MODE1:
			return SPI_CPOL_0;
		case SPI_MODE2:
			return SPI_CPOL_1;
		case SPI_MODE3:
			return SPI_CPOL_1;
	}

	// Error
	return SPI_CPOL_0;
}

inline spi_cpha_t get_cpha(SPIMode mode)
{
	switch (mode)
	{
		case SPI_MODE0:
			return SPI_CPHA_0;
		case SPI_MODE1:
			return SPI_CPHA_1;
		case SPI_MODE2:
			return SPI_CPHA_0;
		case SPI_MODE3:
			return SPI_CPHA_1;
	}

	// Error
	return SPI_CPHA_0;
}

inline spi_inst_t *getDisplaySpi(void)
{
	return TFT_SPI_PORT == 0 ? spi0 : spi1;
}

inline spi_inst_t *getSdSpi(void)
{
	return &SDCARD_SPI == &SPI ? spi0 : spi1;
}

/*******************************************************************************
 * platform.h functions
 *******************************************************************************/

float getCpuSpeedMHz(void)
{
	return clock_get_hz(clk_sys) / 1000000.0f;
}

float getCpuTemperature(void)
{
	return analogReadTemp();
}

SdSpiConfig getSdSpiConfig(void)
{
	return SdSpiConfig(SDCARD_CS_PIN, DEDICATED_SPI, SPI_FULL_SPEED, &sdSpiDriver);
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
	uint rate = spi_get_baudrate(getDisplaySpi());
	if (rate == UINT_MAX)
		rate = SPI_FREQUENCY;
	return rate / 1000000.0f;
}

float getSpiRateSdMHz()
{
	uint rate = spi_get_baudrate(getSdSpi());
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

void setupPlatform(void)
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

		// Sync peripheral clock to CPU clock to get a boost to performance
		uint32_t freq = clock_get_hz(clk_sys);
		clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);

		// Need to reapply SPI baudrate since the multiplier changed
		spi_set_baudrate(getSdSpi(), SPI_FULL_SPEED);
		spi_set_baudrate(getDisplaySpi(), SPI_FREQUENCY);
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

/*******************************************************************************
 * Custom SPI driver implementation
 *******************************************************************************/

static spi_inst_t *sdSpi;
static spi_cpol_t sdCpol;
static spi_cpha_t sdCpha;
static spi_order_t sdBitOrder;

SdSpiDriverT2P::SdSpiDriverT2P()
{
	sdSpi = getSdSpi();
}

void SdSpiDriverT2P::activate()
{
	// SDCARD_SPI.beginTransaction(spiSettings);
}

void SdSpiDriverT2P::begin(SdSpiConfig config)
{
	spi_deinit(sdSpi);

	setSckSpeed(config.maxSck);
	sdCpol = get_cpol(spiSettings.getDataMode());
	sdCpha = get_cpha(spiSettings.getDataMode());
	sdBitOrder = spiSettings.getBitOrder() == MSBFIRST ? SPI_MSB_FIRST : SPI_LSB_FIRST;

	spi_init(sdSpi, spiSettings.getClockFreq());
	spi_set_format(sdSpi, 8, sdCpol, sdCpha, sdBitOrder);

	gpio_set_function(SDCARD_MISO_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SDCARD_MOSI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SDCARD_SCK_PIN, GPIO_FUNC_SPI);

	gpio_pull_up(SDCARD_MISO_PIN); // Pull up MISO

	if (SDCARD_CS_PIN > -1)
	{
		gpio_set_function(SDCARD_CS_PIN, GPIO_FUNC_SPI);
		gpio_pull_up(SDCARD_CS_PIN);
	}
}

void SdSpiDriverT2P::deactivate()
{
	// SDCARD_SPI.endTransaction();
}

uint8_t SdSpiDriverT2P::receive()
{
	uint8_t value;
	spi_read_blocking(sdSpi, 0XFF, &value, 1);
	return value;
}

uint8_t SdSpiDriverT2P::receive(uint8_t *buf, size_t count)
{
	spi_read_blocking(sdSpi, 0xFF, buf, count);
	return 0;
}

void SdSpiDriverT2P::send(uint8_t data)
{
	spi_write_blocking(sdSpi, &data, 1);
}

void SdSpiDriverT2P::send(const uint8_t *buf, size_t count)
{
	spi_write_blocking(sdSpi, buf, count);
}

void SdSpiDriverT2P::setSckSpeed(uint32_t maxSck)
{
	spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
}

#endif
