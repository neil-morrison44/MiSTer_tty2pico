/*******************************************************************************
 * tty2pico for Arduino
 *******************************************************************************/

// Configuration overrides - See "config.h" for more details
// #define WAIT_FOR_SERIAL 1
// #define VERBOSE_OUTPUT 1
// #define SHOW_FPS 1
// #define USE_GIF_BUFFERING

#include "config.h"
#include <Arduino.h>
#include "platform.h"
#include "tty.h"
#include "storage.h"
#include "usbmsc.h"
#include "display.h"
#include "commands.h"

#define POLLING_LOOP_DELAY 500

static bool runLoop1 = false;

void setup()
{
	/* NOTE: Most of these setup functions need to run in a particular order */

	beginUsbMsc();                  // Start up USB MSC interface, must be BEFORE the serial interface so CDC doesn't take over
	setupTTY();                     // Bring up the serial interface
	setupStorage();                 // Configure storage
	setupPlatform();                // Apply platform-specific code for the MCU (tune bus speed, overclock, etc.)
	setupDisplay();                 // Configure and enable the display
	readyUsbMsc();                  // Set USB MSC ready after storage is available
	setupQueue();                   // Set up multicore queue
	setDirectory(config.imagePath); // Set the working image path
	showStartup();
}

void loop()
{
	static String command;
	static CommandData data;
	static uint32_t nextRead;

	uint32_t nextDiff = millis() - nextRead;
	if (nextDiff >= 0)
	{
		command = readTTY();
		nextRead = millis() + POLLING_LOOP_DELAY; // Delay the next read for better performance

		if (command != "")
			runCommand(CommandData::parseCommand(command));
	}

	loopDisplay(millis());
	delay(0); // 0ms delay here gives a very small (and I mean SMALL) performance boost, marginally more than yield()
}
