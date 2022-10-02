#ifndef CONFIG_H
#define CONFIG_H

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
#define STARTUP_LOGO "/logos/menu.png" // The logo to show on startup (when not in slideshow mode)
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

#endif
