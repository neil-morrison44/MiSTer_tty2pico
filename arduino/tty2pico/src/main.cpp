/*******************************************************************************
 * tty2pico for Arduino
 *
 *******************************************************************************/

/*******************************************************************************
 * Configuration overrides - See "config.h" for more details
 *******************************************************************************/
#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL 0 // Wait for serial connection before running program code
#endif
#ifndef VERBOSE_OUTPUT
#define VERBOSE_OUTPUT 0 // Log a lot of stuff to the serial output, only useful for debugging
#endif
// #ifndef STARTUP_LOGO
// #define STARTUP_LOGO "/logos/pattern.gif" // The logo to show on startup (when not in slideshow mode)
// #endif

// #define SLIDESHOW_ON_START 1
// #define BACKGROUND_COLOR 0x0000 // The default background color
// #define OVERCLOCK_RP2040
// #define USE_GIF_BUFFERING

/*******************************************************************************
 * Includes
 *******************************************************************************/
#include "config.h"
#ifdef OVERCLOCK_RP2040
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#endif
#include <Arduino.h>
#include "config.h"
#include "tty.h"
#include "storage.h"
#include "usbmsc.h"
#include "display.h"
#include "commands.h"

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

bool runLoop1 = false;
queue_t cmdQ;
uint32_t nextSerialRead;

void setup()
{
#ifdef OVERCLOCK_RP2040
	// Apply an overclock to 250MHz (2x stock) and voltage tweak to stablize most RP2040 boards.
	// If it's good enough for pixel-pushing in MicroPython, it's good enough for us :P
	// https://github.com/micropython/micropython/issues/8208
	vreg_set_voltage(VREG_VOLTAGE_1_20); // Set voltage to 1.2v
	delay(10); // Allow vreg time to stabilize
	set_sys_clock_khz(250000, true); // Overclock to 250MHz
#endif

	queue_init(&cmdQ, sizeof(CommandData), 1);

	setupTTY();
	setupStorage();
	setupUsbMsc();
	setupDisplay();
	setDirectory(LOGO_PATH);

	runLoop1 = true;
}

void setup1()
{
	// Pause core 1 until setup() is done
	while (!runLoop1) delay(1);

#if !defined(SLIDESHOW_ON_START) || SLIDESHOW_ON_START == 0
	showStartup();
#endif
}

void loop()
{
	static String command;
	static CommandData data;

	loopMSC();

	if (millis() > nextSerialRead)
	{
		command = readTTY();
		if (command != "")
		{
			data = parseCommand(String(command));
			if (data.command != TTY2CMD_NONE && data.command != TTY2CMD_UNKNOWN) // Could do some logging of unknown commands here
				queue_try_add(&cmdQ, &data);
		}
		nextSerialRead = millis() + 500; // Delay the next read for better performance
	}
}

void loop1()
{
	static CommandData data;

	while (queue_try_remove(&cmdQ, &data))
	{
		runCommand(data);
	}

	loopDisplay(millis());
}
