#ifndef DISPLAY_H
#define DISPLAY_H

#define FS_NO_GLOBALS // Need this to fix import issues with LittleFS dependency

#include "config.h"
#include "storage.h"
#include <SPI.h>
#include <AnimatedGIF.h>
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
static uint16_t usImage[TFT_DISPLAY_WIDTH * TFT_DISPLAY_HEIGHT];

// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
void gifDrawLine(GIFDRAW *pDraw)
{
	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, iWidth;

	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > TFT_DISPLAY_WIDTH)
		iWidth = TFT_DISPLAY_WIDTH - pDraw->iX;
	usPalette = pDraw->pPalette;
	if (pDraw->iY + pDraw->y >= TFT_DISPLAY_HEIGHT || pDraw->iX >= TFT_DISPLAY_WIDTH || iWidth < 1)
		return;
	if (pDraw->y == 0) { // start of frame, set address window on LCD
		tft.dmaWait(); // wait for previous writes to complete before trying to access the LCD
		tft.setAddrWindow(xoffset + pDraw->iX, yoffset + pDraw->iY, pDraw->iWidth, pDraw->iHeight);
		// By setting the address window to the size of the current GIF frame, we can just write
		// continuously over the whole frame without having to set the address window again
	}
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
	d = &usImage[pDraw->iWidth * pDraw->y];
	if (pDraw->ucHasTransparency) // if transparency used
	{
		uint8_t c, ucTransparent = pDraw->ucTransparent;
		int x;
		for (x=0; x < iWidth; x++)
		{
			c = *s++;
			if (c != ucTransparent)
				d[x] = usPalette[c];
		}
	}
	else
	{
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		for (x=0; x<iWidth; x++)
		{
			d[x] = usPalette[s[x]];
		}
	}
	tft.dmaWait(); // wait for last write to complete (the last scan line)
	// We write with block set to FALSE (3rd param) so that we can be decoding the next
	// line while the DMA hardware continues to write data to the LCD controller
	tft.pushPixelsDMA(d, iWidth);
}

void showGIF(const char *path)
{
	int16_t rc = gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDrawLine);
	if (rc == GIF_SUCCESS)
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

		gif.begin();
		while (gif.playFrame(true, 0))
		{
			yield();
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
		Serial.print("Could not open GIF "); Serial.print(path); Serial.print(", error code "); Serial.println(rc);
	}
}

void showGIF(String path)
{
	showGIF(path.c_str());
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
	if (pathString.endsWith(".png") || pathString.endsWith(".PNG"))
	{
		showPNG(path);
	}
	else if (pathString.endsWith(".gif") || pathString.endsWith(".GIF"))
	{
		showGIF(path);
	}
}

void showImage(String path)
{
	showImage(path.c_str());
}

#endif
