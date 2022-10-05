/*******************************************************************************
 * tty2pico for Arduino
 *
 *******************************************************************************/

/*******************************************************************************
 * Configuration overrides - See "config.h" for more details
 *******************************************************************************/
#define CONFIG_PICO                       // Valid options: CONFIG_PICO, CONFIG_PICO_SD, CONFIG_ROUNDYPI, CONFIG_THINGPLUS
#define SLIDESHOW_ON_START 0              // Display the slideshow on startup instead of the STARTUP_LOGO
#define WAIT_FOR_SERIAL    0              // Wait for serial connection before running program code
#define VERBOSE_OUTPUT     0              // Log a lot of stuff to the serial output, only useful for debugging
#define JPEGDEC_EXCLUDE_FS                // Hack to exclude the FS.h import in JPEGDEC, conflicts with custom FS stuff for FlashFS
#define USE_DMA                           // Use DMA transfers for display communication
// #define STARTUP_LOGO "/logos/mister.gif" // The logo to show on startup (when not in slideshow mode)

/*******************************************************************************
 * Includes
 *******************************************************************************/
#include "config.h"
#include "tty.h"
#include "storage.h"
#include "display.h"
#include "commander.h"

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

#if defined(STORAGE_TYPE_FLASH_FS)
	loopMSC();
#endif

	if (millis() > nextSerialRead)
	{
		command = readTTY();
		if (command != "")
		{
			Serial.print("TTY: "); Serial.println(command.c_str());
			data = parseCommand(String(command));
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
		Serial.print("Received "); Serial.println(data.commandText);
		runCommand(data);
	}

	loopDisplay(millis());
}
