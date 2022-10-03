#ifndef DISPLAY_H
#define DISPLAY_H

#define FS_NO_GLOBALS // Need this to fix import issues with LittleFS dependency

#include "config.h"
#include "storage.h"
#include <SPI.h>
#include <AnimatedGIF.h>
#include <JPEGDEC.h>
#include <PNGdec.h>
#include <TFT_eSPI.h>

#define DISABLE_COLOR_MIXING 0xffffffff

int16_t xpos = 0;
int16_t ypos = 0;
int32_t xoffset = 0;
int32_t yoffset = 0;

/*******************************************************************************
 * Display setup
 *******************************************************************************/

TFT_eSPI tft = TFT_eSPI();

void setupDisplay()
{
#if defined(TFT_BL)
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, LOW); // Turn off backlight before starting up screen
#endif

	// Setup screen
	tft.init();
#if defined(TFT_ROTATION)
	tft.setRotation(TFT_ROTATION);
#endif
	tft.setTextFont(2);

	// Clear display
	tft.startWrite();
	tft.fillScreen(BACKGROUND_COLOR);
	tft.endWrite();

#if defined(TFT_BL)
	delay(50); // Small delay to avoid garbage output
	digitalWrite(TFT_BL, HIGH); // Turn backlight back on after init
#endif

	Serial.println("Display setup complete");
}

/*******************************************************************************
 * GIF
 *******************************************************************************/

AnimatedGIF gif;

// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
// Draw a line of image directly on the LCD
void gifDrawLine(GIFDRAW *pDraw)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("gifDrawLine");
#endif
	uint8_t *s;
	uint16_t *d, *usPalette, usTemp[320];
	int x, y, iWidth;

	iWidth = pDraw->iWidth;
	if (iWidth > TFT_DISPLAY_WIDTH)
		iWidth = TFT_DISPLAY_WIDTH;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line

	s = pDraw->pPixels;
	if (pDraw->ucDisposalMethod == 2) // restore to background color
	{
		for (x=0; x<iWidth; x++)
		{
			if (s[x] == pDraw->ucTransparent)
				s[x] = pDraw->ucBackground;
		}
		pDraw->ucHasTransparency = 0;
	}

	// Apply the new pixels to the main image
	if (pDraw->ucHasTransparency) // if transparency used
	{
		uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
		int x, iCount;
		pEnd = s + iWidth;
		x = 0;
		iCount = 0; // count non-transparent pixels
		while(x < iWidth)
		{
			c = ucTransparent-1;
			d = usTemp;
			while (c != ucTransparent && s < pEnd)
			{
				c = *s++;
				if (c == ucTransparent) // done, stop
				{
					s--; // back up to treat it like transparent
				}
				else // opaque
				{
						*d++ = usPalette[c];
						iCount++;
				}
			} // while looking for opaque pixels
			if (iCount) // any opaque pixels?
			{
				tft.pushRect(pDraw->iX + x + xoffset, y + yoffset, iCount, 1, (uint16_t*)usTemp);
				x += iCount;
				iCount = 0;
			}
			// no, look for a run of transparent pixels
			c = ucTransparent;
			while (c == ucTransparent && s < pEnd)
			{
				c = *s++;
				if (c == ucTransparent)
						iCount++;
				else
						s--;
			}
			if (iCount)
			{
				x += iCount; // skip these
				iCount = 0;
			}
		}
	}
	else
	{
		s = pDraw->pPixels;
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		for (x=0; x<iWidth; x++)
			usTemp[x] = usPalette[*s++];

		tft.pushRect(pDraw->iX, y, iWidth, 1, (uint16_t*)usTemp);
	}
}

void showGIF(const char *path)
{
	if (gif.open(path, &gifOpen, &gifClose, &gifRead, &gifSeek, &gifDrawLine))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened GIF "); Serial.print(path); Serial.print(" with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		xoffset = (TFT_DISPLAY_WIDTH - gif.getCanvasWidth()) / 2;
		yoffset = (TFT_DISPLAY_HEIGHT - gif.getCanvasHeight()) / 2;

		tft.fillScreen(BACKGROUND_COLOR);
		tft.startWrite();

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		long runtime = micros();
		int frames = 0;
#endif

		gif.begin(BIG_ENDIAN_PIXELS);
		while (gif.playFrame(true, 0))
		{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
			frames++;
#endif
		}
		gif.close();
		tft.endWrite();

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		runtime = micros() - runtime;
		Serial.print("Ran GIF "); Serial.print(path); Serial.print(" at "); Serial.print(frames / (runtime / 1000000.0)); Serial.println(" fps");
#endif
	}
	else
	{
		Serial.print("Could not open GIF "); Serial.println(path);
	}
}

void showGIF(String path)
{
	showGIF(path.c_str());
}

/*******************************************************************************
 * JPEG
 *******************************************************************************/

JPEGDEC jpeg;

int jpegDraw(JPEGDRAW *pDraw)
{
	tft.startWrite();
	// tft.pushImageDMA(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
	tft.pushRect(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
	tft.endWrite();
	return 1;
}

void showJPEG(const char *path)
{
	if (jpeg.open(path, &jpegOpen, &jpegClose, &jpegRead, &jpegSeek, &jpegDraw))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened JPEG "); Serial.print(path); Serial.print(" with resolution "); Serial.print(jpeg.getWidth()); Serial.print(" x "); Serial.println(jpeg.getHeight());
#endif

		xoffset = (TFT_DISPLAY_WIDTH - jpeg.getWidth()) / 2;
		yoffset = (TFT_DISPLAY_HEIGHT - jpeg.getHeight()) / 2;

		tft.startWrite();
		tft.fillScreen(BACKGROUND_COLOR);
		jpeg.setPixelType(RGB565_BIG_ENDIAN);
		jpeg.decode(xoffset, yoffset, 0);
		jpeg.close();
		tft.endWrite();
	}
	else
	{
		Serial.print("Could not open JPEG "); Serial.println(path);
	}
}

void showJPEG(String path)
{
	showJPEG(path.c_str());
}

/*******************************************************************************
 * PNG
 *******************************************************************************/

PNG png;

void pngDrawLine(PNGDRAW *pDraw)
{
	uint16_t lineBuffer[MAX_IMAGE_WIDTH];
	png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, DISABLE_COLOR_MIXING);
	tft.pushImage(xpos + xoffset, ypos + pDraw->y + yoffset, pDraw->iWidth, 1, lineBuffer);
}

void showPNG(const char *path)
{
	int16_t rc = png.open(path, &pngOpen, &pngClose, &pngRead, &pngSeek, &pngDrawLine);
	if (rc == PNG_SUCCESS)
	{
		xoffset = (TFT_DISPLAY_WIDTH - png.getWidth()) / 2;
		yoffset = (TFT_DISPLAY_HEIGHT - png.getHeight()) / 2;

		tft.fillScreen(BACKGROUND_COLOR);
		tft.startWrite();
		rc = png.decode(NULL, 0);
		tft.endWrite();
	}
}

void showPNG(String path)
{
	showPNG(path.c_str());
}

/*******************************************************************************
 * Generic image functions
 *******************************************************************************/

void showImage(const char *path)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Showing image: "); Serial.println(path);
#endif

	String pathString = String(path);
	pathString.trim();

	if (pathString.endsWith(".jpg") || pathString.endsWith(".JPG") || pathString.endsWith(".jpeg") || pathString.endsWith(".JPEG"))
		showJPEG(path);
	else if (pathString.endsWith(".png") || pathString.endsWith(".PNG"))
		showPNG(path);
	else if (pathString.endsWith(".gif") || pathString.endsWith(".GIF"))
		showGIF(path);
}

void showImage(String path)
{
	showImage(path.c_str());
}

#endif
