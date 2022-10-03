#ifndef COMMANDER_H
#define COMMANDER_H

#include "config.h"
#include "display.h"

static String lastCommand;

static void cmdSaver(String command)
{
	bool wasRunning = isSlideshowActive();
	if (command.indexOf(',') > -1 && command.length() > 9)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		Serial.print("Screensaver mode changed to "); Serial.println(mode);
		setSlideshowActive(mode != "0");

		if (wasRunning && !isSlideshowActive())
			showImage(STARTUP_LOGO);
	}
	else
	{
		setSlideshowActive(true); // Default to turning on slideshow
	}
}

static void cmdSetCore(String command)
{
	setSlideshowActive(false);

	String coreName;
	if (command.startsWith("CMDCOR"))
	{
		int coreIndex = command.indexOf(',') + 1;
		coreName = command.substring(coreIndex, command.length());
	}
	else coreName = command;

	String path = LOGO_PATH + coreName + ".png";
	Serial.print("Loading png file: "); Serial.println(path.c_str());
	showImage(path);
}

static void cmdSetTime(void)
{
	// TODO: Add for ESP32?
}

static void cmdShow(String command)
{
	String path = command.substring(command.indexOf(',') + 1);
	showImage(path);
}

static void cmdUnknown()
{
	// Unknown command, don't do anything
}

void processCommand(String command)
{
	if (command != "" && command != lastCommand)
	{
		lastCommand = command;

		if (command.startsWith("CMDSETTIME"))                                 cmdSetTime();
		else if (command.startsWith("CMDSAVER"))                              cmdSaver(command);
		else if (command.startsWith("CMDSHOW,"))                              cmdShow(command);
		else if (command.startsWith("CMD") && !command.startsWith("CMDCOR,")) cmdUnknown();
		else                                                                  cmdSetCore(command); // Assume core name if no command was matched
	}
}

#endif
