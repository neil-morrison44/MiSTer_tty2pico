#ifndef SDCARD_H
#define SDCARD_H

#define ENABLE_EXTENDED_TRANSFER_CLASS 1
#define FAT12_SUPPORT 1

#include "../config.h"
#include <SPI.h>
#include <SD.h>
#include <PNGdec.h>

/*************************
 * Helper functions
 *************************/

File getFile(const char *path, oflag_t oflag = O_RDONLY)
{
  File sdFile;

  Serial.print("Checking file exists: "); Serial.println(path);
	if (!SD.exists(path))
  {
    Serial.println("File not found");
		return sdFile;
  }

  Serial.println("File exists");
  sdFile.close(); // Ensure any previous file has been closed
  Serial.println("Previous file closed");
  sdFile = SD.open(path, oflag);
  Serial.print("Opened file: "); Serial.println(sdFile.name());
  return sdFile;
}

File getFile(String path, oflag_t oflag = O_RDONLY)
{
  return getFile(path.c_str(), oflag);
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

/*************************
 * PNG functions
 *************************/

static File pngfile;

void *pngOpen(const char *filename, int32_t *size)
{
  Serial.printf("Attempting to open %s\n", filename);
  pngfile = getFile(filename);
  *size = pngfile.size();
  return &pngfile;
}

void pngClose(void *handle)
{
  if (handle == nullptr)
    return;
    
  File pngfile = *((File*)handle);
  pngfile.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length)
{
  if (!pngfile.available())
    return 0;

  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
  if (!pngfile.available())
    return 0;

  page = page; // Avoid warning
  return pngfile.seek(position);
}

#endif
