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
// #define STARTUP_LOGO "/logos/mister.gif" // The logo to show on startup (when not in slideshow mode)
// #endif
#include "config.h"

/*******************************************************************************
 * Includes
 *******************************************************************************/
#include <Arduino.h>
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
	// Configure peripheral clock source to allow increased max SPI frequency when RP2040 is overclocked.
	// This also gives a slight performance boost to SPI transfers for the display when not overclocking.
	// Referenced here: https://github.com/Bodmer/TFT_eSPI/issues/1460#issuecomment-1006661452
	uint32_t freq = clock_get_hz(clk_sys);
	clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);

	queue_init(&cmdQ, sizeof(CommandData), 1);

	setupTTY();
	setupStorage();
	setupUsbMsc();
	setupDisplay();

	setDirectory(LOGO_PATH);

#if !defined(SLIDESHOW_ON_START) || SLIDESHOW_ON_START == 0
	showStartup();
#endif

	runLoop1 = true;
}

void setup1()
{
	// Pause core 1 until setup() is done
	while (!runLoop1) delay(1);
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
