#ifndef COMMANDER_H
#define COMMANDER_H

#include "config.h"
#include "display.h"

static String lastCommand;

static void cmdBye(void)
{
	showMister();
}

static void cmdSaver(String command)
{
	DisplayState lastDisplayState = getDisplayState();
	if (command.indexOf(',') > -1 && command.length() > 9)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		Serial.print("Screensaver mode changed to "); Serial.println(mode);
		bool enabled = mode != "0";
		if (enabled)
			setDisplayState(DISPLAY_SLIDESHOW);
		if (!enabled && lastDisplayState != DISPLAY_SLIDESHOW)
			 showStartup();
	}
	else
	{
		setDisplayState(DISPLAY_SLIDESHOW);
	}
}

static void cmdSetCore(String command)
{
	String coreName;
	if (command.startsWith("CMDCOR"))
	{
		int coreIndex = command.indexOf(',') + 1;
		coreName = command.substring(coreIndex, command.length());
	}
	else coreName = command;

	String path;
	bool found = false;
	for (int i = 0; i < imageExtensionCount; i++)
	{
		path = LOGO_PATH + coreName + String(imageExtensions[i]);
		File file = getFile(path);
		if (file)
		{
			found = true;
			Serial.print("Loading "); Serial.println(path.c_str());
			break;
		}
	}

	if (found)
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

		if (command == "CMDBYE")                                              cmdBye();
		else if (command.startsWith("CMDSETTIME"))                            cmdSetTime();
		else if (command.startsWith("CMDSAVER"))                              cmdSaver(command);
		else if (command.startsWith("CMDSHOW,"))                              cmdShow(command);
		else if (command.startsWith("CMD") && !command.startsWith("CMDCOR,")) cmdUnknown();
		else                                                                  cmdSetCore(command); // Assume core name if no command was matched
	}
}

#endif
