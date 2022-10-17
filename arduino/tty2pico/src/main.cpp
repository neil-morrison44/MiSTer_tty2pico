/*******************************************************************************
 * tty2pico for Arduino
 *******************************************************************************/

// Configuration overrides - See "config.h" for more details
#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL 0 // Wait for serial connection before running program code
#endif
#ifndef VERBOSE_OUTPUT
#define VERBOSE_OUTPUT 1 // Log a lot of stuff to the serial output, only useful for debugging
#endif
// #define USE_GIF_BUFFERING

#include "config.h"
#include <Arduino.h>
#include "platform.h"
#include "tty.h"
#include "storage.h"
#include "usbmsc.h"
#include "display.h"
#include "commands.h"

static bool runLoop1 = false;

void setup()
{
	setupUsbMsc();
	setupTTY();
	setupStorage();
	setupCPU();
	setupDisplay();
	setupQueue();

	setDirectory(config.imagePath);

	runLoop1 = true;
}

void setup1()
{
	// Pause core 1 until setup() is done
	while (!runLoop1) delay(1);
	showStartup();
}

void loop()
{
	static String command;
	static CommandData data;
	static uint32_t nextRead;

	if (millis() > nextRead)
	{
		command = readTTY();
		if (command != "")
		{
			data = CommandData::parseCommand(command);
			addToQueue(data);
		}
		nextRead = millis() + 500; // Delay the next read for better performance
	}

	loopMSC();

	delay(5); // 5ms delay here seems to increase FPS in testing
}

void loop1()
{
	loopQueue();
	loopDisplay(millis());

	delay(0); // 0ms delay here gives a very small, and I mean SMALL, performance boost
}
