#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Arduino.h>

// When adding a new command do the following:
// * Add a `const String [CMDNAME]` variable to commands.h
// * Add to TTY2CMD enum to commands.h
// * Add or use existing function for handling command, prefix function name with `cmd`
// * Add handling to `parseCommand()` and `runCommand()`
// * Document where appropriate

// tty2oled commands
const String CMDBYE     = "CMDBYE";
const String CMDCLS     = "CMDCLS";
const String CMDCORE    = "CMDCOR";
const String CMDDOFF    = "CMDDOFF";
const String CMDDON     = "CMDDON";
const String CMDENOTA   = "CMDENOTA";
const String CMDROT     = "CMDROT";
const String CMDSAVER   = "CMDSAVER";
const String CMDSETTIME = "CMDSETTIME";
const String CMDSHSYSHW = "CMDSHSYSHW";
const String CMDSHTEMP  = "CMDSHTEMP";
const String CMDSNAM    = "CMDSNAM";
const String CMDSORG    = "CMDSORG";
const String CMDSWSAVER = "CMDSWSAVER";
const String CMDTEST    = "CMDTEST";
const String CMDTXT     = "CMDTXT";

// tty2pico commands
const String CMDGETSYS  = "CMDGETSYS";
const String CMDGETTIME = "CMDGETTIME";
const String CMDSHOW    = "CMDSHOW";
const String CMDUSBMSC  = "CMDUSBMSC";

typedef enum TTY2CMD {
	TTY2CMD_NONE = 0,
	TTY2CMD_CLS,
	TTY2CMD_COR,
	TTY2CMD_BYE,
	TTY2CMD_DOFF,
	TTY2CMD_DON,
	TTY2CMD_ENOTA,
	TTY2CMD_GETSYS,
	TTY2CMD_GETTIME,
	TTY2CMD_ROT,
	TTY2CMD_SAVER,
	TTY2CMD_SETTIME,
	TTY2CMD_SHOW,
	TTY2CMD_SHSYSHW,
	TTY2CMD_SHTEMP,
	TTY2CMD_SNAM,
	TTY2CMD_SORG,
	TTY2CMD_SWSAVER,
	TTY2CMD_TEST,
	TTY2CMD_TXT,
	TTY2CMD_UNKNOWN,
	TTY2CMD_USBMSC,
} TTY2CMD;

struct CommandData
{
	CommandData() { }
	CommandData(TTY2CMD command) : command(command) { }
	CommandData(TTY2CMD command, String commandText) : command(command), commandText(commandText) { }

	static CommandData parseCommand(String command)
	{
		if (command != "")
		{
			if      (command.startsWith(CMDBYE))                                  return CommandData(TTY2CMD_BYE, command);
			else if (command.startsWith(CMDCLS))                                  return CommandData(TTY2CMD_CLS, command);
			else if (command.startsWith(CMDDOFF))                                 return CommandData(TTY2CMD_DOFF, command);
			else if (command.startsWith(CMDDON))                                  return CommandData(TTY2CMD_DON, command);
			else if (command.startsWith(CMDENOTA))                                return CommandData(TTY2CMD_ENOTA, command);
			else if (command.startsWith(CMDGETSYS))                               return CommandData(TTY2CMD_GETSYS, command);
			else if (command.startsWith(CMDGETTIME))                              return CommandData(TTY2CMD_GETTIME, command);
			else if (command.startsWith(CMDROT))                                  return CommandData(TTY2CMD_ROT, command);
			else if (command.startsWith(CMDSAVER))                                return CommandData(TTY2CMD_SAVER, command);
			else if (command.startsWith(CMDSETTIME))                              return CommandData(TTY2CMD_SETTIME, command);
			else if (command.startsWith(CMDSHOW))                                 return CommandData(TTY2CMD_SHOW, command);
			else if (command.startsWith(CMDSHSYSHW))                              return CommandData(TTY2CMD_SHSYSHW, command);
			else if (command.startsWith(CMDSHTEMP))                               return CommandData(TTY2CMD_SHTEMP, command);
			else if (command.startsWith(CMDSNAM))                                 return CommandData(TTY2CMD_SNAM, command);
			else if (command.startsWith(CMDSORG))                                 return CommandData(TTY2CMD_SORG, command);
			else if (command.startsWith(CMDSWSAVER))                              return CommandData(TTY2CMD_SWSAVER, command);
			else if (command.startsWith(CMDTEST))                                 return CommandData(TTY2CMD_TEST, command);
			else if (command.startsWith(CMDTXT))                                  return CommandData(TTY2CMD_TXT, command);
			else if (command.startsWith(CMDUSBMSC))                               return CommandData(TTY2CMD_USBMSC, command);
			else if (command.startsWith("CMD") && !command.startsWith(CMDCORE))   return CommandData(TTY2CMD_UNKNOWN, command);
			else    /* Assume core name if no command was matched */              return CommandData(TTY2CMD_COR, command);
		}

		return CommandData(TTY2CMD_NONE, command);
	}

	TTY2CMD command;
	String commandText;
};



typedef enum DateTimeFormat {
	DTF_UNIX = 0,
	DTF_HUMAN,
} DateTimeFormat;

typedef enum DisplayState {
	DISPLAY_ANIMATED_GIF,
	DISPLAY_ANIMATED_GIF_LOOPING,
	DISPLAY_MISTER,
	DISPLAY_RANDOM_SHAPES,
	DISPLAY_SLIDESHOW,
	DISPLAY_STATIC_IMAGE,
	DISPLAY_STATIC_TEXT,
	DISPLAY_SYSTEM_INFORMATION,
} DisplayState;

#endif
