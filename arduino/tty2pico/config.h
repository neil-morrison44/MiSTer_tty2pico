#ifndef CONFIG_H
#define CONFIG_H

#include <User_Setup_Select.h> // This will pull in the TFT_eSPI configuration defines for the display

/**************************
 * Board configuration
 **************************/

#if defined(CONFIG_ROUNDYPI)
#include "configs/roundypi.h"
#elif defined(CONFIG_THINGPLUS)
#include "configs/thingplus.h"
#else
#include "configs/pico.h"
#endif

/**************************
 * Software configuration
 **************************/

#ifndef TTY_BAUDRATE
#define TTY_BAUDRATE 115200 // Set the baud rate of the serial connection
#endif

#ifndef LOGO_PATH
#define LOGO_PATH "/logos/" // Path to logo folder
#endif

#ifndef STARTUP_LOGO
#define STARTUP_LOGO "/logos/mister.png" // The logo to show on startup (when not in slideshow mode)
#endif

#ifndef BACKGROUND_COLOR
#define BACKGROUND_COLOR 0x0000 // The default background color
#endif

#ifndef SLIDESHOW_DELAY
#define SLIDESHOW_DELAY 2000 // The time between slideshow changes
#endif

#ifndef SLIDESHOW_ON_START
#define SLIDESHOW_ON_START 0 // Display the slideshow on startup instead of the STARTUP_LOGO
#endif

#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL 0 // Wait for serial connection before running program code
#endif

#ifndef VERBOSE_OUTPUT
#define VERBOSE_OUTPUT 0 // Log a lot of stuff to the serial output, only useful for debugging
#endif

#ifndef TFT_ROTATION
#define TFT_ROTATION 0 // Set the rotation position, values are from 0-3
#endif

#ifndef DISK_LABEL
#define DISK_LABEL "tty2pico" // Default label for newly formatted filesystem, limit of 11 characters
#endif

// If defined will use the DMA mode with TFT_eSPI library
// This can live in either the board-specific config, or at the top of the .ino file
// #define USE_DMA

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

#ifndef MAX_IMAGE_WIDTH
#define MAX_IMAGE_WIDTH TFT_DISPLAY_WIDTH // This value is used to allocate line buffers, so usually set to your display width
#endif

#endif
