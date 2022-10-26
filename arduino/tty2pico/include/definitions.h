#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Arduino.h>
#include "SPI.h"
#include "FsLib/FsVolume.h"
#include "FsLib/FsFile.h"
#include "SpiDriver/SdSpiDriver.h"

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
		String bigcmd = command;
		bigcmd.toUpperCase();

		if (bigcmd != "")
		{
			if      (bigcmd.startsWith(CMDBYE))                                  return CommandData(TTY2CMD_BYE, command);
			else if (bigcmd.startsWith(CMDCLS))                                  return CommandData(TTY2CMD_CLS, command);
			else if (bigcmd.startsWith(CMDDOFF))                                 return CommandData(TTY2CMD_DOFF, command);
			else if (bigcmd.startsWith(CMDDON))                                  return CommandData(TTY2CMD_DON, command);
			else if (bigcmd.startsWith(CMDENOTA))                                return CommandData(TTY2CMD_ENOTA, command);
			else if (bigcmd.startsWith(CMDGETSYS))                               return CommandData(TTY2CMD_GETSYS, command);
			else if (bigcmd.startsWith(CMDGETTIME))                              return CommandData(TTY2CMD_GETTIME, command);
			else if (bigcmd.startsWith(CMDROT))                                  return CommandData(TTY2CMD_ROT, command);
			else if (bigcmd.startsWith(CMDSAVER))                                return CommandData(TTY2CMD_SAVER, command);
			else if (bigcmd.startsWith(CMDSETTIME))                              return CommandData(TTY2CMD_SETTIME, command);
			else if (bigcmd.startsWith(CMDSHOW))                                 return CommandData(TTY2CMD_SHOW, command);
			else if (bigcmd.startsWith(CMDSHSYSHW))                              return CommandData(TTY2CMD_SHSYSHW, command);
			else if (bigcmd.startsWith(CMDSHTEMP))                               return CommandData(TTY2CMD_SHTEMP, command);
			else if (bigcmd.startsWith(CMDSNAM))                                 return CommandData(TTY2CMD_SNAM, command);
			else if (bigcmd.startsWith(CMDSORG))                                 return CommandData(TTY2CMD_SORG, command);
			else if (bigcmd.startsWith(CMDSWSAVER))                              return CommandData(TTY2CMD_SWSAVER, command);
			else if (bigcmd.startsWith(CMDTEST))                                 return CommandData(TTY2CMD_TEST, command);
			else if (bigcmd.startsWith(CMDTXT))                                  return CommandData(TTY2CMD_TXT, command);
			else if (bigcmd.startsWith("CMD") && !bigcmd.startsWith(CMDCORE))    return CommandData(TTY2CMD_UNKNOWN, command);
			else    /* Assume core name if no command was matched */             return CommandData(TTY2CMD_COR, command);
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

class SdSpiDriverT2P : public SdSpiBaseClass
{
public:
	SdSpiDriverT2P();

	// Activate SPI hardware with correct speed and mode.
	void activate();

	// Initialize the SPI bus.
	void begin(SdSpiConfig config);
	// Deactivate SPI hardware.
	void deactivate();
	// Receive a byte.
	uint8_t receive();
	// Receive multiple bytes.
	uint8_t receive(uint8_t *buf, size_t count);
	// Send a byte.
	void send(uint8_t data);
	// Send multiple bytes.
	void send(const uint8_t *buf, size_t count);
	// Save SPISettings for new max SCK frequency
	void setSckSpeed(uint32_t maxSck);

private:
	SPISettings spiSettings;
};

class FsVolumeTS;

class FsFileTS
{
public:
	FsFileTS() { }
	FsFileTS(File32 file) : file32(file) { }
	FsFileTS(FsFile file) : fsFile(file) { }
	~FsFileTS()
	{
		if (fsFile) fsFile.close();
		if (file32) file32.close();
	}

	explicit operator bool() const { return file32 || fsFile; }

	static void setActiveVolume(FsVolumeTS *volume) { activeVolume = volume; }

	bool available(void);
	bool close(void);
	uint8_t getError() const;
	size_t getName(char* name, size_t len);
	bool isDir(void);
	FsFileTS openNextFile(void);
	bool openNext(FsBaseFile* dir, oflag_t oflag);
	uint64_t position(void);
	size_t print(const char *str);
	int read(void* buf, size_t count);
	void rewindDirectory(void);
	bool seek(uint64_t position);
	uint64_t size(void);
	size_t write(const void* buf, size_t count);

private:
	static FsVolumeTS *activeVolume;
	File32 file32;
	FsFile fsFile;
};

class FsVolumeTS
{
public:
	FsVolumeTS() { }
	FsVolumeTS(FatVolume *vol) : flashVol(vol) { }
	FsVolumeTS(FsVolume *vol) : sdVol(vol) { }
	~FsVolumeTS()
	{
		flashVol = nullptr;
		sdVol = nullptr;
	}

	explicit operator bool() const { return flashVol || sdVol; }

	bool exists(const char *path);
	FsFileTS open(const char *path, oflag_t oflag);

	FatVolume *getFlashVol(void) { return flashVol; }
	FsVolume *getSdVol(void) { return sdVol; }

private:
	FatVolume *flashVol;
	FsVolume *sdVol;
};

#endif
