/*******************************************************************************
 * tty2pico for Arduino
 *
 *******************************************************************************/

/*******************************************************************************
 * Configuration overrides - See "config.h" for more details
 *******************************************************************************/
#define CONFIG_PICO           // Valid options: CONFIG_PICO, CONFIG_ROUNDYPI, CONFIG_THINGPLUS
#define SLIDESHOW_ON_START 0  // Display the slideshow on startup instead of the STARTUP_LOGO
#define WAIT_FOR_SERIAL    0  // Wait for serial connection before running program code
#define VERBOSE_OUTPUT     1  // Log a lot of stuff to the serial output, only useful for debugging
#define JPEGDEC_EXCLUDE_FS    // Hack to exclude the FS.h import in JPEGDEC, conflicts with custom FS stuff for FlashFS

/*******************************************************************************
 * Includes
 *******************************************************************************/
#include "config.h"
#include "tty.h"
#include "storage.h"
#include "display.h"
#include "commander.h"
#include "slideshow.h"

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/
void setup()
{
	setupTTY();
	setupDisplay();
  setupStorage();

  setDirectory(LOGO_PATH);

#if !defined(SLIDESHOW_ON_START) || SLIDESHOW_ON_START == 0
  showImage(STARTUP_LOGO);
#endif
}

void loop()
{
#if defined(STORAGE_TYPE_FLASH_FS)
	loopMSC();
#endif

  String command = readTTY();
  processCommand(command);

  loopSlideshow(millis());
}
