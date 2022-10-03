#ifndef SDCARD_H
#define SDCARD_H

#define ENABLE_EXTENDED_TRANSFER_CLASS 1
#define FAT12_SUPPORT 1

#include "../config.h"
#include <SPI.h>
#include <SD.h>

/*************************
 * Helper functions
 *************************/

File getFile(const char *path, oflag_t oflag)
{
	File file;

	if (!SD.exists(path))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("File not found: "); Serial.println(path);
#endif
		return file;
	}

	file.close(); // Ensure any previous file has been closed
	file = SD.open(path, oflag);
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Opened file: "); Serial.println(file.name());
#endif
	return file;
}

/*************************
 * Setup functions
 *************************/

void setupStorage(void)
{
	SDCARD_SPI.setRX(SDCARD_MISO_PIN);
	SDCARD_SPI.setTX(SDCARD_MOSI_PIN);
	SDCARD_SPI.setSCK(SDCARD_SCK_PIN);

	if (!SD.begin(SDCARD_CS_PIN, SPI_FULL_SPEED, SDCARD_SPI))
	{
		Serial.println("initialization failed!");
		return;
	}

	Serial.println("SD storage setup complete");
}

/*************************
 * Directory functions
 *************************/

static File dir;

String getDirectory(void)
{
	return String(dir.fullName());
}

int getFileCount(void)
{
	dir.rewindDirectory();

	int count = 0;
	while (true)
	{
		File entry = dir.openNextFile();
		if (entry)
			count += 1;
		else
			break;
	}

	dir.rewindDirectory();
	return count;
}

String getNextFile(void)
{
	static File entry;

	entry = dir.openNextFile();
	if (!entry)
		return "";

	return String(entry.fullName());
}

void printDirectory(const char *path, int numTabs)
{
	File dir = getFile(path);

	while (true)
	{
		File entry =  dir.openNextFile();
		if (!entry) // no more files
			break;

		for (uint8_t i = 0; i < numTabs; i++)
			Serial.print('\t');

		Serial.print(entry.name());
		if (entry.isDirectory())
		{
			Serial.println("/");
			printDirectory(entry.name(), numTabs + 1);
		}
		else
		{
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.print(entry.size(), DEC);
			time_t cr = entry.getCreationTime();
			time_t lw = entry.getLastWrite();
			struct tm *tmstruct = localtime(&cr);
			Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
			tmstruct = localtime(&lw);
			Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
		}
		entry.close();
	}
}

void rewindDirectory(void)
{
	dir.rewindDirectory();
}

void setDirectory(String path)
{
	dir.close();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Setting directory to: "); Serial.println(path.c_str());
#endif
	dir = getFile(path);
}

#endif
