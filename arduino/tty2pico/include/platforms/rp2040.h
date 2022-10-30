#ifndef TTY2PICO_PLATFORM_IMPL_H
#define TTY2PICO_PLATFORM_IMPL_H

#include "config.h"
#include "definitions.h"
#include "SPI.h"
#include "api/HardwareSPI.h"
#include "SpiDriver/SdSpiDriver.h"
#include "TFT_eSPI.h"
#include "hardware/dma.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <UnixTime.h>

#define SYS_CHANGE_DELAY 250

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
#ifdef TFT_SPI_PORT
	return TFT_SPI_PORT == 0 ? spi0 : spi1;
#else
	return nullptr;
#endif
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
	// Initial config at compatibility speeds.
	// When setupPlatform() is called the speed will be automatically set.
#if ENABLE_DEDICATED_SPI == 1
	return SdSpiConfig(SDCARD_CS_PIN, DEDICATED_SPI, SPI_HALF_SPEED, &sdSpiDriver);
#else
	return SdSpiConfig(SDCARD_CS_PIN, SHARED_SPI, SPI_HALF_SPEED, &sdSpiDriver);
#endif
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

// Returns SPI rate, or PIO clock rate when RP2040_PIO_SPI is enabled
float getSpiRateDisplayMHz()
{
#if RP2040_PIO_SPI == 1
	// PIO write frequency from TFT_eSPI lib = (CPU clock/(4 * DIV_UNITS))
	float pioClock = clock_get_hz(clk_sys) / (4.0f * DIV_UNITS);
	return pioClock / 1000000.0f;
#else
	uint rate = spi_get_baudrate(getDisplaySpi());
	if (rate == UINT_MAX)
		rate = 0;

	return rate / 1000000.0f;
#endif
}

float getSpiRateSdMHz()
{
	uint rate = spi_get_baudrate(getSdSpi());
	if (rate == UINT_MAX)
		rate = 0;

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
	if (rtc_get_datetime(&datetime))
	{
		memset(datetimeString, 0, sizeof(datetimeString));
		datetime_to_str(datetimeString, sizeof(datetimeString), &datetime);
		Serial.print("RTC date and time set to "); Serial.println(datetimeString);
	}
	else
	{
		Serial.print("Invalid timestamp: "); Serial.println(timestamp);
	}
}

void resetForUpdate(void)
{
	reset_usb_boot(0, 0);
}

void setupPlatform(bool (*checkSDCallback)())
{
	rp2040.enableDoubleResetBootloader();

	// Apply an overclock for about 2x performance and a voltage tweak to stablize most RP2040 boards.
	// If it's good enough for pixel-pushing in MicroPython, it's good enough for us :P
	// https://github.com/micropython/micropython/issues/8208
	int cpuMHz = config.cpuBoost ? 266 : 250;
	vreg_voltage voltage = config.cpuBoost ? VREG_VOLTAGE_MAX : VREG_VOLTAGE_1_20;
	vreg_set_voltage(voltage);
	delay(SYS_CHANGE_DELAY); // Allow vreg time to stabilize
	if (!set_sys_clock_khz(cpuMHz * 1000, false))
	{
		Serial.println("Couldn't overclock CPU, halting...");
		while (1) delay(1);
	}
	Serial.print("CPU overclocked to "); Serial.print(cpuMHz); Serial.println("MHz");

	// Sync peripheral bus to the CPU speed, gives boost to max SPI rate
	uint32_t freq = clock_get_hz(clk_sys);
	clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);
	Serial.println("Peripheral bus frequencies applied");

#if RP2040_PIO_SPI != 1
	spi_set_baudrate(getDisplaySpi(), SPI_FREQUENCY);
	Serial.println("Display frequency set");
#endif

	if (checkSDCallback())
	{
		Serial.println("Setting SD card SPI rate");
		bool tryNormalSpeed = true;
		unsigned long speed;

		if (config.sdMode)
		{
			Serial.print("Trying to set SD mode to "); Serial.println(config.sdMode);
			speed = SD_SCK_MHZ(SD_MODE_SPEEDS[config.sdMode]);
			spi_set_baudrate(getSdSpi(), speed);
			delay(SYS_CHANGE_DELAY);
			if (checkSDCallback())
			{
				sdSpiDriver.setSckSpeed(speed);
				tryNormalSpeed = false;
			}
			else
			{
				Serial.println("Couldn't set SD to configured SD mode, falling back to full speed");
			}
		}

		// Normal setup or fallback for failed SD rate OC
		if (tryNormalSpeed)
		{
			config.sdMode = SD_MODE_DEFAULT;
			speed = SD_SCK_MHZ(SD_MODE_SPEEDS[config.sdMode]);
			spi_set_baudrate(getSdSpi(), speed);
			delay(SYS_CHANGE_DELAY);
			if (checkSDCallback())
				sdSpiDriver.setSckSpeed(speed);
			else
				Serial.println("Couldn't set SD to normal speed, SD will be disabled");
		}

		if (checkSDCallback())
		{
			Serial.print("SD reader speed set to "); Serial.print(getSpiRateSdMHz()); Serial.println("MHz");
		}
		else
		{
			Serial.print("Check your SD card format and/or SD reader wiring and power requirements");
		}
	}

	Serial.println("Platform setup complete");
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

inline void pauseBackground(void)
{
	noInterrupts();
	rp2040.idleOtherCore();
}

inline void resumeBackground(void)
{
	rp2040.resumeOtherCore();
	interrupts();
}

/*******************************************************************************
 * Custom SD SPI driver implementation
 *******************************************************************************/

static spi_inst_t *sdSpi;
static spi_cpol_t sdCpol;
static spi_cpha_t sdCpha;
static spi_order_t sdBitOrder;

#if USE_DMA_SD == 1
#define SD_DMA_BUFFER_SIZE 2048 // Each direction gets a 2k buffer
static int sdRxChannel;
static int sdTxChannel;
static uint8_t sdRxBuf[SD_DMA_BUFFER_SIZE];
static uint8_t sdTxBuf[SD_DMA_BUFFER_SIZE];
#endif

SdSpiDriverT2P::SdSpiDriverT2P()
{
	sdSpi = getSdSpi();
}

void SdSpiDriverT2P::activate()
{
	(void)0;
}

void SdSpiDriverT2P::begin(SdSpiConfig config)
{
	// SPI configuration
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

#if USE_DMA_SD == 1
	// DMA configuration - 2 channels (TX/RX)
	sdRxChannel = dma_claim_unused_channel(true);
	dma_channel_config rxConfig = dma_channel_get_default_config(sdRxChannel);
	channel_config_set_transfer_data_size(&rxConfig, DMA_SIZE_8);
	channel_config_set_dreq(&rxConfig, spi_get_dreq(sdSpi, false));
	channel_config_set_read_increment(&rxConfig, false);
	dma_channel_configure(
		sdRxChannel,            // Channel to be configured
		&rxConfig,              // The configuration we just created
		sdRxBuf,                // The initial write address
		&spi_get_hw(sdSpi)->dr, // The initial read address
		SD_DMA_BUFFER_SIZE,     // Element count (each element is of size transfer_data_size)
		false                   // Don't start immediately.
	);

	sdTxChannel = dma_claim_unused_channel(true);
	dma_channel_config txConfig = dma_channel_get_default_config(sdTxChannel);
	channel_config_set_transfer_data_size(&txConfig, DMA_SIZE_8);
	channel_config_set_dreq(&txConfig, spi_get_dreq(sdSpi, true));
	channel_config_set_write_increment(&txConfig, false);
	dma_channel_configure(
		sdTxChannel,            // Channel to be configured
		&txConfig,              // The configuration we just created
		&spi_get_hw(sdSpi)->dr, // The initial write address
		sdTxBuf,                // The initial read address
		SD_DMA_BUFFER_SIZE,     // Element count (each element is of size transfer_data_size)
		false                   // Don't start immediately.
	);

	dma_start_channel_mask((1u << sdTxChannel) | (1u << sdRxChannel));
	Serial.println("SD DMAs started");
#endif
}

void SdSpiDriverT2P::deactivate()
{
#if USE_DMA_SD == 1
	dma_channel_unclaim(sdRxChannel);
	dma_channel_unclaim(sdTxChannel);
#else
	(void)0;
#endif
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
	spiSettings = SPISettings(maxSck, spiSettings.getBitOrder(), spiSettings.getDataMode());
}

/*******************************************************************************
 * Custom file access implementation
 *******************************************************************************/

FsVolumeTS *FsFileTS::activeVolume = nullptr;

bool FsVolumeTS::exists(const char *path)
{
	if (sdVol)
	{
		pauseBackground();
		bool exists = sdVol && sdVol->exists(path);
		resumeBackground();
		return exists;
	}
	else return flashVol->exists(path);
}

FsFileTS FsVolumeTS::open(const char *path, oflag_t oflag)
{
	if (sdVol)
	{
		FsFile tmpFile;
		pauseBackground();
		bool opened = sdVol && tmpFile.open(sdVol, path, oflag);
		resumeBackground();
		return opened ? FsFileTS(tmpFile) : FsFileTS();
	}
	else return FsFileTS(flashVol->open(path, oflag));
}

bool FsFileTS::available(void)
{
	if (fsFile)
	{
		pauseBackground();
		int available = fsFile ? fsFile.available() : 0;
		resumeBackground();
		return available;
	}
	else return file32.available();
}

bool FsFileTS::close(bool sync)
{
	if (fsFile)
	{
		pauseBackground();
		bool closed = fsFile ? fsFile.close() : true;
		if (sync)
			fsFile.sync();
		resumeBackground();
		return closed;
	}
	else
	{
		bool closed = file32.close();
		if (sync)
			file32.sync();
		return closed;
	}
}

uint8_t FsFileTS::getError() const
{
	if (fsFile)
	{
		pauseBackground();
		uint8_t error = fsFile ? fsFile.getError() : 0;
		resumeBackground();
		return error;
	}
	else return file32.getError();
}

size_t FsFileTS::getName(char *name, size_t len)
{
	if (fsFile)
	{
		pauseBackground();
		size_t size = fsFile ? fsFile.getName(name, len) : 0;
		resumeBackground();
		return size;
	}
	else return file32.getName(name, len);
}

bool FsFileTS::isDir(void)
{
	if (fsFile)
	{
		pauseBackground();
		bool isDir = fsFile && fsFile.isDir();
		resumeBackground();
		return isDir;
	}
	else return file32.isDir();
}

FsFileTS FsFileTS::openNextFile(void)
{
	if (fsFile)
	{
		pauseBackground();
		FsFileTS nextFile = fsFile ? FsFileTS(fsFile.openNextFile()) : FsFileTS();
		resumeBackground();
		return nextFile;
	}
	else return FsFileTS(file32.openNextFile());
}

bool FsFileTS::openNext(FsBaseFile *dir, oflag_t oflag)
{
	if (fsFile)
	{
		pauseBackground();
		bool opened = fsFile && fsFile.openNext(dir, oflag);
		resumeBackground();
		return opened;
	}
	else
	{
		char dirName[255];
		size_t dirNameLength = dir->getName(dirName, 255);
		FatFile tmpFile = activeVolume->getFlashVol()->open(dirName, oflag);
		return file32.openNext(&tmpFile, oflag);
	}
}

uint64_t FsFileTS::position(void)
{
	if (fsFile)
	{
		pauseBackground();
		uint64_t pos = fsFile ? fsFile.position() : 0;
		resumeBackground();
		return pos;
	}
	else return file32.position();
}

size_t FsFileTS::print(const char *str)
{
	if (fsFile)
	{
		pauseBackground();
		size_t byteCount = fsFile ? fsFile.print(str) : 0;
		resumeBackground();
		return byteCount;
	}
	else return file32.print(str);
}

int FsFileTS::read(void *buf, size_t count)
{
	if (fsFile)
	{
		pauseBackground();
		int byteCount = fsFile ? fsFile.read(buf, count) : 0;
		resumeBackground();
		return byteCount;
	}
	else return file32.read(buf, count);
}

void FsFileTS::rewindDirectory(void)
{
	if (fsFile)
	{
		pauseBackground();
		if (fsFile) fsFile.rewindDirectory();
		resumeBackground();
	}
	else file32.rewindDirectory();
}

bool FsFileTS::seek(uint64_t position)
{
	if (fsFile)
	{
		pauseBackground();
		bool success = fsFile && fsFile.seek(position);
		resumeBackground();
		return success;
	}
	else return file32.seek(position);
}

uint64_t FsFileTS::size(void)
{
	if (fsFile)
	{
		pauseBackground();
		uint64_t fileSize = fsFile ? fsFile.size() : 0;
		resumeBackground();
		return fileSize;
	}
	else return file32.size();
}

size_t FsFileTS::write(const void* buf, size_t count)
{
	if (fsFile)
	{
		pauseBackground();
		size_t byteCount = fsFile ? fsFile.write(buf, count) : 0;
		resumeBackground();
		return byteCount;
	}
	else return file32.write(buf, count);
}

#endif
