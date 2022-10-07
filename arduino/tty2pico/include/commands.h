#ifndef COMMANDER_H
#define COMMANDER_H

#include "config.h"
#include "display.h"
#include "storage.h"

typedef enum TTY2CMD {
	TTY2CMD_NONE = 0,
	TTY2CMD_COR,
	TTY2CMD_BYE,
	TTY2CMD_SAVER,
	TTY2CMD_SHOW,
	TTY2CMD_SETTIME,
	TTY2CMD_UNKNOWN,
} TTY2CMD;

struct CommandData
{
	CommandData() { }
	CommandData(TTY2CMD command) : command(command) { }
	CommandData(TTY2CMD command, String commandText) : command(command), commandText(commandText) { }

	TTY2CMD command;
	String commandText;
};

static bool cmdBye(void)
{
	showMister();
	return true;
}

static bool cmdSaver(String command)
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

	return true;
}

static bool cmdSetCore(String command)
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
		path = LOGO_PATH + coreName + String(imageExtensions[i]);;
		if (fileExists(path))
		{
			found = true;
			Serial.print("Loading "); Serial.println(path.c_str());
			break;
		}
	}

	if (found)
		showImage(path);

	return found;
}

static bool cmdSetTime(String command)
{
	(void)command;

	// TODO: Add for ESP32?
	return true;
}

static bool cmdShow(String command)
{
	String path = command.substring(command.indexOf(',') + 1);
	showImage(path);
	return true;
}

static bool cmdUnknown()
{
	// Unknown command, don't do anything
	return true;
}

CommandData parseCommand(String command)
{
	if (command != "")
	{
		if      (command == "CMDBYE")                                           return CommandData(TTY2CMD_BYE);
		else if (command.startsWith("CMDSETTIME"))                              return CommandData(TTY2CMD_SETTIME, command);
		else if (command.startsWith("CMDSAVER"))                                return CommandData(TTY2CMD_SAVER, command);
		else if (command.startsWith("CMDSHOW,"))                                return CommandData(TTY2CMD_SHOW, command);
		else if (command.startsWith("CMD") && !command.startsWith("CMDCOR,"))   return CommandData(TTY2CMD_UNKNOWN, command);
		else    /* Assume core name if no command was matched */                return CommandData(TTY2CMD_COR, command);
	}

	return CommandData(TTY2CMD_NONE, command);
}

bool runCommand(CommandData data)
{
	switch (data.command)
	{
		case TTY2CMD_BYE:     return cmdBye();
		case TTY2CMD_COR:     return cmdSetCore(data.commandText);
		case TTY2CMD_SAVER:   return cmdSaver(data.commandText);
		case TTY2CMD_SETTIME: return cmdSetTime(data.commandText);
		case TTY2CMD_SHOW:    return cmdShow(data.commandText);
		case TTY2CMD_NONE:    return true;

		case TTY2CMD_UNKNOWN:
			Serial.print("Received unknown command: ");
			Serial.println(data.commandText);
			return false;

		default:
			Serial.print("Unrecognized TTY2CMD command: ");
			Serial.println(data.command);
			return false;
	}
}

#endif
