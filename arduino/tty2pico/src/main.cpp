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

void setup()
{
	/* NOTE: Most of these setup functions need to run in a particular order */
	setupTTY();           // Bring up the serial interface
	setupStorage();       // Configure storage
	setupUsbMsc();        // Set up as USB Mass Storage Device
	setupPlatform(hasSD); // Apply platform-specific code for the MCU (tune bus speed, overclock, etc.)
	setupDisplay();       // Configure and enable the display
	setupQueue();         // Set up task queue

	setDirectory(config.imagePath); // Set the working image path
	showStartup();
}

void loop()
{
	static String command;
	static CommandData data;
	static uint32_t nextRead;
	static uint32_t now;

	now = millis();

	if (now - nextRead > 0)
	{
		command = readTTY();
		nextRead = now + POLLING_LOOP_DELAY; // Delay the next read for better performance

		if (command != "")
		{
			data = CommandData::parseCommand(command);
			addToQueue(data);
			loopQueue();
		}
	}

	loopDisplay(now);
}
