#ifndef DISPLAY_H
#define DISPLAY_H

#define FS_NO_GLOBALS // Need this to fix import issues with LittleFS dependency

#include "config.h"
#include "storage.h"
#include <SPI.h>
#include <PNGdec.h>
#include <TFT_eSPI.h>

#define MAX_IMAGE_WIDTH TFT_WIDTH // Adjust for your images
#define DISABLE_COLOR_MIXING 0xffffffff

int16_t xpos = 0;
int16_t ypos = 0;
int32_t xoffset = 0;
int32_t yoffset = 0;
PNG png;
TFT_eSPI tft = TFT_eSPI();

void setupDisplay()
{
	tft.init();
#if defined(TFT_ROTATION)
	tft.setRotation(TFT_ROTATION);
#endif
	tft.fillScreen(BACKGROUND_COLOR);
	tft.setTextFont(2);

	Serial.println("Display setup complete");
}

void pngDraw(PNGDRAW *pDraw)
{
	uint16_t lineBuffer[MAX_IMAGE_WIDTH];
	png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, DISABLE_COLOR_MIXING);
	tft.pushImage(xpos + xoffset, ypos + pDraw->y + yoffset, pDraw->iWidth, 1, lineBuffer);
}

void showPng(const char *path)
{
	int16_t rc = png.open(path, &pngOpen, &pngClose, &pngRead, &pngSeek, &pngDraw);
	if (rc == PNG_SUCCESS)
	{
		xoffset = (TFT_WIDTH - png.getWidth()) / 2;
		yoffset = (TFT_HEIGHT - png.getHeight()) / 2;

		tft.fillScreen(BACKGROUND_COLOR);
		tft.startWrite();
		rc = png.decode(NULL, 0);
		tft.endWrite();
	}
}

void showPng(String path)
{
	showPng(path.c_str());
}

#endif
