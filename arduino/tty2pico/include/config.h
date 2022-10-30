#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <stdint.h>

#include "tomlcpp.hpp"
#include "utils.h"

#ifndef CONFIG_FILE_PATH
#define CONFIG_FILE_PATH "/tty2pico.toml"  // Path to tty2pico config file
#endif

#ifndef RESERVED_FLASH_KB
#define RESERVED_FLASH_KB \
	512	 // The amount of flash (in KB) reserved for program code
#endif

#ifndef DISK_LABEL
#define DISK_LABEL \
	"TTY2PICO"	// Default label for newly formatted filesystem, limit of 11
				// characters
#endif

#ifndef VERBOSE_OUTPUT
#define VERBOSE_OUTPUT \
	0  // Log a lot of stuff to the serial output, only useful for debugging
#endif

#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL \
	0  // Wait for serial connection before running program code
#endif

/**************************
 * Software configuration
 **************************/

#ifndef LOGO_PATH
#define LOGO_PATH "/logos/"	 // Path to logo folder
#endif

#ifndef STARTUP_DELAY
#define STARTUP_DELAY 5000	// The amount of time to display the startup screen
#endif

#ifndef STARTUP_LOGO
#define STARTUP_LOGO \
	""	// The logo to show on startup (when not in slideshow mode)
#endif

#ifndef BACKGROUND_COLOR
// Predefined values from TFT_eSPI:
// BLACK       0x0000      /*   0,   0,   0 */
// NAVY        0x000F      /*   0,   0, 128 */
// DARKGREEN   0x03E0      /*   0, 128,   0 */
// DARKCYAN    0x03EF      /*   0, 128, 128 */
// MAROON      0x7800      /* 128,   0,   0 */
// PURPLE      0x780F      /* 128,   0, 128 */
// OLIVE       0x7BE0      /* 128, 128,   0 */
// LIGHTGREY   0xD69A      /* 211, 211, 211 */
// DARKGREY    0x7BEF      /* 128, 128, 128 */
// BLUE        0x001F      /*   0,   0, 255 */
// GREEN       0x07E0      /*   0, 255,   0 */
// CYAN        0x07FF      /*   0, 255, 255 */
// RED         0xF800      /* 255,   0,   0 */
// MAGENTA     0xF81F      /* 255,   0, 255 */
// YELLOW      0xFFE0      /* 255, 255,   0 */
// WHITE       0xFFFF      /* 255, 255, 255 */
// ORANGE      0xFDA0      /* 255, 180,   0 */
// GREENYELLOW 0xB7E0      /* 180, 255,   0 */
// PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
// BROWN       0x9A60      /* 150,  75,   0 */
// GOLD        0xFEA0      /* 255, 215,   0 */
// SILVER      0xC618      /* 192, 192, 192 */
// SKYBLUE     0x867D      /* 135, 206, 235 */
// VIOLET      0x915C      /* 180,  46, 226 */
#define BACKGROUND_COLOR 0x0000	 // The default background color
#endif

#ifndef SLIDESHOW_DELAY
#define SLIDESHOW_DELAY 2000  // The time between slideshow changes
#endif

#ifndef TFT_ROTATION
#define TFT_ROTATION 0	// Set the rotation position, values are from 0-3
#endif

#ifndef SHOW_FPS
#define SHOW_FPS 0
#endif

#ifndef USE_DMA
#define USE_DMA \
	0  // If defined will use the DMA mode with TFT_eSPI library for better
	   // performance on supported hardware
#endif

#ifndef USE_DMA_SD
#define USE_DMA_SD \
	0  // If defined will use the DMA mode with SD card for better
	   // performance on supported hardware
#endif

#ifndef CPU_BOOST
#define CPU_BOOST \
	1  // Enable a slight boost over the standard overclock (if available
	   // for platform), will have slight effect SD SPI rate
#endif

#ifndef SD_MODE
#define SD_MODE 0  // See TTY2PICO_SdModes for details
#endif

/**************************
 * Computed configuration
 **************************/

#ifdef TFT_DISPLAY_WIDTH
#undef TFT_DISPLAY_WIDTH
#endif
#ifndef TFT_DISPLAY_WIDTH
#if TFT_ROTATION % 2 == 0
#define TFT_DISPLAY_WIDTH TFT_WIDTH
#else
#define TFT_DISPLAY_WIDTH TFT_HEIGHT
#endif
#endif

#ifdef TFT_DISPLAY_HEIGHT
#undef TFT_DISPLAY_HEIGHT
#endif
#ifndef TFT_DISPLAY_HEIGHT
#if TFT_ROTATION % 2 == 0
#define TFT_DISPLAY_HEIGHT TFT_HEIGHT
#else
#define TFT_DISPLAY_HEIGHT TFT_WIDTH
#endif
#endif

#if TFT_DISPLAY_HEIGHT > TFT_DISPLAY_WIDTH
#define TFT_DISPLAY_MAX TFT_DISPLAY_HEIGHT
#else
#define TFT_DISPLAY_MAX TFT_DISPLAY_WIDTH
#endif

/**************************
 * Config data handling
 **************************/

typedef enum TTY2PICO_Font {
	// GFXFF =  FreeFonts. Include access to the 48 Adafruit_GFX free fonts
	// FF1
	// to
	// FF48 and custom fonts
	GLCD = 1,	// Font 1. Original Adafruit 8 pixel font needs ~1820 bytes
				// in FLASH
	FONT2 = 2,	// Font 2. Small 16 pixel high font, needs ~3534 bytes in
				// FLASH, 96 characters
	FONT4 = 4,	// Font 4. Medium 26 pixel high font, needs ~5848 bytes in
				// FLASH,
				// 96 characters
	FONT6 = 6,	// Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH,
				// only characters 1234567890:-.apm
	FONT7 = 7,	// Font 7. 7 segment 48 pixel font, needs ~2438 bytes in
				// FLASH, only characters 1234567890:.
	FONT8 = 8,	// Font 8. Large 75 pixel font needs ~3256 bytes in FLASH,
				// only characters 1234567890:-.
} TTY2PICO_Font;

typedef enum TTY2PICO_SdModes {
	SD_MODE_DEFAULT = 0,  // 25.0MHz CPU@250MHz | 26.7MHz CPU@266MHz (1:1
						  // peripheral clock)
	SD_MODE_HIGH,		  // 32.2MHz CPU@250MHz | 33.3MHz CPU@266MHz (1:1
						  // peripheral clock)
	SD_MODE_MAX,  // 41.3MHz CPU@250MHz | 44.3MHz CPU@266MHz (1:1 peripheral
				  // clock)
} TTY2PICO_SdModes;

const int SD_MODE_SPEEDS[] = {
	27,	 // SD_SPEED_DEFAULT - 25.0MHz CPU@250MHz | 26.7MHz CPU@266MHz (1:1
		 // peripheral clock)
	34,	 // SD_SPEED_HIGH    - 32.2MHz CPU@250MHz | 33.3MHz CPU@266MHz (1:1
		 // peripheral clock)
	45,	 // SD_SPEED_MAX     - 41.3MHz CPU@250MHz | 44.3MHz CPU@266MHz (1:1
		 // peripheral clock)
};

struct TTY2PICO_Config {
	uint16_t backgroundColor = BACKGROUND_COLOR;
	String imagePath = LOGO_PATH;
	bool cpuBoost = CPU_BOOST;
	TTY2PICO_SdModes sdMode = (TTY2PICO_SdModes)SD_MODE;
	String startupCommand = "";
	unsigned long startupDelay = STARTUP_DELAY;
	String startupImage = STARTUP_LOGO;
	int slideshowDelay = SLIDESHOW_DELAY;
	String slideshowFolder = LOGO_PATH;
	uint8_t tftRotation = TFT_ROTATION;
	int tftWidth = TFT_WIDTH;
	int tftHeight = TFT_HEIGHT;
	bool uncapFramerate = false;
	bool showDefaultForUnknown = false;

	int getDisplayHeight() const {
		return tftHeight < tftWidth ? tftHeight : tftWidth;
	}
	int getDisplayWidth() const {
		return tftWidth > tftHeight ? tftWidth : tftHeight;
	}
	int getMidpointX() const { return getDisplayWidth() / 2; }
	int getMidpointY() const { return getDisplayHeight() / 2; }
	int getLineBufferSize() const { return getDisplayWidth(); }
	int getFontSmall() const { return getDisplayHeight() < 160 ? GLCD : FONT2; }
	int getFontLarge() const {
		return getDisplayHeight() < 160 ? FONT2 : FONT4;
	}
	int getFontSmallSize() const { return getDisplayHeight() < 160 ? 8 : 16; }
	int getFontLargeSize() const { return getDisplayHeight() < 160 ? 16 : 26; }
};

TTY2PICO_Config config;

// Parses a tty2pico config from memory. Will return an error message if failed.
const char *parseConfig(char *buffer) {
	// Do we have a config to parse?
	if (buffer == nullptr || sizeof(buffer) == 0)
		return "No config data present";

	// Trim any trailing whitespace
	trimTrailing(buffer);

	auto res = toml::parse(buffer);
	if (!res.table) return res.errmsg.c_str();

	auto tty2pico = res.table->getTable("tty2pico");
	if (!tty2pico)
		return "Invalid configuration file, missing [tty2pico] config "
			   "section";

	auto [backgroundColorOK, backgroundColor] =
		tty2pico->getInt("backgroundColor");
	if (backgroundColorOK) config.backgroundColor = (uint16_t)backgroundColor;

	auto [cpuBoostOK, cpuBoost] = tty2pico->getBool("cpuBoost");
	if (cpuBoostOK) config.cpuBoost = cpuBoost;

	auto [imagePathOK, imagePath] = tty2pico->getString("imagePath");
	if (imagePathOK) config.imagePath = imagePath.c_str();

	auto [sdModeOK, sdMode] = tty2pico->getInt("sdMode");
	if (sdModeOK)
		config.sdMode = (sdMode >= SD_MODE_DEFAULT && sdMode <= SD_MODE_MAX)
							? (TTY2PICO_SdModes)sdMode
							: SD_MODE_DEFAULT;

	auto [slideshowDelayOK, slideshowDelay] =
		tty2pico->getInt("slideshowDelay");
	if (slideshowDelayOK) config.slideshowDelay = (uint32_t)slideshowDelay;

	auto [slideshowFolderOK, slideshowFolder] =
		tty2pico->getString("slideshowFolder");
	if (slideshowFolderOK) config.slideshowFolder = slideshowFolder.c_str();

	auto [startupCommandOK, startupCommand] =
		tty2pico->getString("startupCommand");
	if (startupCommandOK && startupCommand.length() > 0)
		config.startupCommand = startupCommand.c_str();

	auto [startupDelayOK, startupDelay] = tty2pico->getInt("startupDelay");
	if (startupDelayOK) config.startupDelay = (unsigned long)startupDelay;

	auto [startupImageOK, startupImage] = tty2pico->getString("startupImage");
	if (startupImageOK) config.startupImage = startupImage.c_str();

	auto [tftWidthOK, tftWidth] = tty2pico->getInt("tftWidth");
	if (tftWidthOK) config.tftWidth = (uint16_t)tftWidth;

	auto [tftHeightOK, tftHeight] = tty2pico->getInt("tftHeight");
	if (tftHeightOK) config.tftHeight = (uint16_t)tftHeight;

	auto [tftRotationOK, tftRotation] = tty2pico->getInt("tftRotation");
	if (tftRotationOK) config.tftRotation = (uint8_t)tftRotation;

	auto [uncapFramerateOK, uncapFramerate] =
		tty2pico->getBool("uncapFramerate");
	if (uncapFramerateOK) config.uncapFramerate = uncapFramerate;

	auto [showDefaultForUnknownOK, showDefaultForUnknown] =
		tty2pico->getBool("showDefaultForUnknown");
	if (showDefaultForUnknownOK)
		config.showDefaultForUnknown = showDefaultForUnknown;

	return nullptr;
}

int exportConfig(char *buffer, int bufferSize) {
	String commandText =
		String("title = \"tty2pico Configuration\"\n") + "\n[tty2pico]" +
		"\nbackgroundColor = " + String(config.backgroundColor) +
		"\ncpuBoost = " + String(config.cpuBoost ? "true" : "false") +
		"\nimagePath = \"" + config.imagePath + "\"" +
		"\nsdMode = " + String(config.sdMode) + "\nstartupCommand = \"" +
		config.startupCommand + "\"" +
		"\nstartupDelay = " + String(config.startupDelay) +
		"\nstartupImage = \"" + config.startupImage + "\"" +
		"\nslideshowDelay = " + String(config.slideshowDelay) +
		"\nslideshowFolder = \"" + config.slideshowFolder + "\"" +
		(TFT_WIDTH != config.tftWidth
			 ? "\ntftWidth = " + String(config.tftWidth)
			 : "") +
		(TFT_HEIGHT != config.tftHeight
			 ? "\ntftHeight = " + String(config.tftHeight)
			 : "") +
		(TFT_ROTATION != config.tftRotation
			 ? "\ntftRotation = " + String(config.tftRotation)
			 : "") +
		"\nuncapFramerate = " +
		String(config.uncapFramerate ? "true" : "false") +
		"\nshowDefaultForUnknown = " +
		String(config.showDefaultForUnknown ? "true" : "false");

	memcpy(buffer, commandText.c_str(), commandText.length());
	return commandText.length();
}

#endif
