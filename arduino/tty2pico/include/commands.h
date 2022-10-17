#ifndef COMMANDER_H
#define COMMANDER_H

#include "config.h"
#include <string>
#include "pico/bootrom.h"
#include "display.h"
#include "storage.h"
#include "usbmsc.h"

using namespace std;

// When adding a new command do the following:
// * Add a `const static String [CMDNAME]` variable for the command text
// * Add to TTY2CMD enum
// * Add or use existing function for handling command, prefix function name with `cmd`
// * Add handling to `parseCommand()` and `runCommand()`
// * Document where appropriate

// tty2oled commands
const static String CMDBYE     = "CMDBYE";
const static String CMDCLS     = "CMDCLS";
const static String CMDCORE    = "CMDCOR";
const static String CMDDOFF    = "CMDDOFF";
const static String CMDDON     = "CMDDON";
const static String CMDENOTA   = "CMDENOTA";
const static String CMDROT     = "CMDROT";
const static String CMDSAVER   = "CMDSAVER";
const static String CMDSETTIME = "CMDSETTIME";
const static String CMDSHSYSHW = "CMDSHSYSHW";
const static String CMDSHTEMP  = "CMDSHTEMP";
const static String CMDSNAM    = "CMDSNAM";
const static String CMDSORG    = "CMDSORG";
const static String CMDSWSAVER = "CMDSWSAVER";
const static String CMDTEST    = "CMDTEST";
const static String CMDTXT     = "CMDTXT";

// tty2pico commands
const static String CMDGETSYS  = "CMDGETSYS";
const static String CMDSHOW    = "CMDSHOW";
const static String CMDUSBMSC  = "CMDUSBMSC";

typedef enum TTY2CMD {
	TTY2CMD_NONE = 0,
	TTY2CMD_CLS,
	TTY2CMD_COR,
	TTY2CMD_BYE,
	TTY2CMD_DOFF,
	TTY2CMD_DON,
	TTY2CMD_ENOTA,
	TTY2CMD_GETSYS,
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

	TTY2CMD command;
	String commandText;
};

static String coreName;

static void cmdBye(void)
{
	showMister();
}

static void cmdCls(void)
{
	clearDisplay();
}

static void cmdDisplayOff(void)
{
	digitalWrite(TFT_BL, LOW);
}

static void cmdDisplayOn(void)
{
	digitalWrite(TFT_BL, HIGH);
}

static void cmdEnableOTA()
{
	Serial.println("Restarting in firmware update mode");
	reset_usb_boot(0, 0);
}

static void cmdGetSysInfo()
{
	string info = string("version:")  + string(TTY2PICO_VERSION)
	            + string("|board:")   + string(TTY2PICO_BOARD)
	            + string("|display:") + string(TTY2PICO_DISPLAY);

	Serial.println(info.c_str());
}

static void cmdRotate(String command)
{
	if (command.indexOf(',') > -1)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		int rotationMode = -1;
		if (mode == "0")
			rotationMode = 0;
		else if (mode == "1")
			rotationMode = 2;
		else if (mode == "2")
			rotationMode = 1;
		else if (mode == "3")
			rotationMode = 3;

		if (rotationMode < 0)
		{
			Serial.print("Invalid rotation mode: "); Serial.println(mode);
			return;
		}

#ifdef TFT_ROTATION
		// Rotating needs to factor in the original rotation from TFT_ROTATION
		rotationMode = (rotationMode + TFT_ROTATION) % 4;
#endif
		tft.setRotation(rotationMode);
		config.tftRotation = rotationMode;
		saveConfig();
		Serial.print("Rotation changed to "); Serial.println(mode);
	}
	else Serial.println("No rotation command found");
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
			showStartup(); // Reset to startup image for now, might want to restore state later
	}
	else
	{
		setDisplayState(DISPLAY_SLIDESHOW);
	}
}

static void cmdSetCore(String command)
{
	if (command.startsWith(CMDCORE))
		coreName = command.substring(command.indexOf(',') + 1, command.length());
	else
		coreName = command;

	String path;
	bool found = false;
	for (int i = 0; i < imageExtensionCount; i++)
	{
		// Check for animated file(s) first
		path = config.imagePath + coreName + String(imageExtensions[i]);
#if VERBOSE_OUTPUT == 1
	Serial.print("Checking for file "); Serial.println(path);
#endif
		found = fileExists(path);
		if (found)
			break;
	}

	if (found)
	{
		Serial.print("Loading "); Serial.println(path.c_str());
		showImage(path);
	}
	else
	{
		Serial.print("Couldn't find core display file for "); Serial.println(coreName);
		showText(coreName);
	}
}

static void cmdSetTime(String command)
{
	(void)command;
	// TODO: Add for ESP32?
}

static void cmdShow(String command)
{
	String path = command.substring(command.indexOf(',') + 1);
	showImage(path);
}

static void cmdShowCoreName(void)
{
	showText(coreName);
}

static void cmdShowSystemInfo(void)
{
	showSystemInfo();
}

static void cmdTest(void)
{
	showText("Starting test in..."); delay(2000);
	showText("3"); delay(1000);
	showText("2"); delay(1000);
	showText("1"); delay(1000);
	showSystemInfo();
	delay(3000);
	showGIF((uint8_t *)mister_kun_blink, sizeof(mister_kun_blink));
	drawDemoShapes(5000);
	showText("Test complete!");
	delay(3000);
}

static void cmdText(String command)
{
	String displayText;
	if (command.startsWith(CMDTXT))
		displayText = command.substring(command.indexOf(',') + 1, command.length());
	else
		displayText = command;

	showText(displayText);
}

static void cmdUnknown(String command)
{
#if VERBOSE_OUTPUT == 1
	Serial.print("Received unknown command: "); Serial.println(command);
#endif
	showText(command);
}

static void cmdUsbMsc()
{
	if (!getMscReady())
	{
		showText("Starting USB MSC mode, this could take a while for SD...");
		readyUsbMsc();
	}
}

CommandData parseCommand(String command)
{
	if (command != "")
	{
		if      (command.startsWith(CMDBYE))                                  return CommandData(TTY2CMD_BYE, command);
		else if (command.startsWith(CMDCLS))                                  return CommandData(TTY2CMD_CLS, command);
		else if (command.startsWith(CMDDOFF))                                 return CommandData(TTY2CMD_DOFF, command);
		else if (command.startsWith(CMDDON))                                  return CommandData(TTY2CMD_DON, command);
		else if (command.startsWith(CMDENOTA))                                return CommandData(TTY2CMD_ENOTA, command);
		else if (command.startsWith(CMDGETSYS))                               return CommandData(TTY2CMD_GETSYS, command);
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

void runCommand(CommandData data)
{
	switch (data.command)
	{
		case TTY2CMD_BYE:     return cmdBye();
		case TTY2CMD_CLS:     return cmdCls();
		case TTY2CMD_COR:     return cmdSetCore(data.commandText);
		case TTY2CMD_DOFF:    return cmdDisplayOff();
		case TTY2CMD_DON:     return cmdDisplayOn();
		case TTY2CMD_ENOTA:   return cmdEnableOTA();
		case TTY2CMD_GETSYS:  return cmdGetSysInfo();
		case TTY2CMD_ROT:     return cmdRotate(data.commandText);
		case TTY2CMD_SAVER:   return cmdSaver(data.commandText);
		case TTY2CMD_SETTIME: return cmdSetTime(data.commandText);
		case TTY2CMD_SHSYSHW: return cmdShowSystemInfo();
		case TTY2CMD_SWSAVER: return cmdSaver(data.commandText);
		case TTY2CMD_SHOW:    return cmdShow(data.commandText);
		case TTY2CMD_SHTEMP:  return cmdShowSystemInfo();
		case TTY2CMD_SNAM:    return cmdShowCoreName();
		case TTY2CMD_SORG:    return cmdShowSystemInfo();
		case TTY2CMD_TEST:    return cmdTest();
		case TTY2CMD_TXT:     return cmdText(data.commandText);
		case TTY2CMD_UNKNOWN: return cmdUnknown(data.commandText);
		case TTY2CMD_USBMSC:  return cmdUsbMsc();
		case TTY2CMD_NONE:    return;

		// If you get here you're missing an enum definition ^^^
		default:
			Serial.print("Unrecognized TTY2CMD command: ");
			Serial.println(data.command);
			return;
	}
}

#endif
