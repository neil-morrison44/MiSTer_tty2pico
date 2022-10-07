#ifndef CONFIG_H
#define CONFIG_H

#include <User_Setup_Select.h> // This will pull in the TFT_eSPI configuration defines for the display

/**************************
 * Software configuration
 **************************/

#ifndef TTY_BAUDRATE
#define TTY_BAUDRATE 115200 // Set the baud rate of the serial connection
#endif

#ifndef LOGO_PATH
#define LOGO_PATH "/logos/" // Path to logo folder
#endif

// #ifndef STARTUP_LOGO
// #define STARTUP_LOGO "/logos/mister.gif" // The logo to show on startup (when not in slideshow mode)
// #endif

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
#define BACKGROUND_COLOR 0x7BEF // The default background color
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
#define DISK_LABEL "TTY2PICO" // Default label for newly formatted filesystem, limit of 11 characters
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
