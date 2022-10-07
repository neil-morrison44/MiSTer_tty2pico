#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "storage.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include <JPEGDEC.h>
#include <PNGdec.h>
#include "mister.h"

#define DISABLE_COLOR_MIXING 0xffffffff

typedef enum DisplayState {
	DISPLAY_STATIC_IMAGE,
	DISPLAY_ANIMATED_GIF,
	DISPLAY_SLIDESHOW,
	DISPLAY_MISTER,
} DisplayState;

const char *imageExtensions[] = {
	".gif",
	".png",
	".jpg",
	".jpeg",
};
const int imageExtensionCount = sizeof(imageExtensions) / sizeof(imageExtensions[0]);

int16_t xpos = 0;
int16_t ypos = 0;
int32_t xoffset = 0;
int32_t yoffset = 0;
String currentImage;
#if defined(SLIDESHOW_ON_START) && SLIDESHOW_ON_START == 1
static DisplayState displayState = DISPLAY_SLIDESHOW;
#else
static DisplayState displayState = DISPLAY_STATIC_IMAGE;
#endif

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
#ifdef USE_DMA
	tft.initDMA();
#endif
#if defined(TFT_ROTATION)
	tft.setRotation(TFT_ROTATION);
#endif
	tft.setTextFont(2);

	// Clear display
	tft.fillScreen(TFT_BLACK);

#if defined(TFT_BL)
	delay(50); // Small delay to avoid garbage output
	digitalWrite(TFT_BL, HIGH); // Turn backlight back on after init
#endif

	Serial.println("Display setup complete");
}

/*******************************************************************************
 * GIF
 *******************************************************************************/

static AnimatedGIF gif;

#define TFT_BUFFER_SIZE TFT_DISPLAY_WIDTH

#ifdef USE_DMA
static uint16_t usTemp[2][TFT_BUFFER_SIZE]; // Global display buffer for DMA use
#else
static uint16_t usTemp[1][TFT_BUFFER_SIZE]; // Global display buffer
#endif
static bool dmaBuf = 0;

// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
// Draw a line of image directly on the LCD
void gifDrawLine(GIFDRAW *pDraw)
{
	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, y, iWidth, iCount;

	// Display bounds check and cropping
	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > TFT_DISPLAY_WIDTH)
		iWidth = TFT_DISPLAY_WIDTH - pDraw->iX;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line
	if (y >= TFT_DISPLAY_HEIGHT || pDraw->iX >= TFT_DISPLAY_WIDTH || iWidth < 1)
		return;

	// Old image disposal
	s = pDraw->pPixels;
	if (pDraw->ucDisposalMethod == 2) // restore to background color
	{
		for (x = 0; x < iWidth; x++)
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
		pEnd = s + iWidth;
		x = 0;
		iCount = 0; // count non-transparent pixels
		while (x < iWidth)
		{
			c = ucTransparent - 1;
			d = &usTemp[0][0];
			while (c != ucTransparent && s < pEnd && iCount < TFT_BUFFER_SIZE)
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
				// DMA would degrtade performance here due to short line segments
				tft.setAddrWindow(pDraw->iX + x + xoffset, y + yoffset, iCount, 1);
				tft.pushPixels(usTemp, iCount);
				x += iCount;
				iCount = 0;
			}
			// no, look for a run of transparent pixels
			c = ucTransparent;
			while (c == ucTransparent && s < pEnd)
			{
				c = *s++;
				if (c == ucTransparent)
					x++;
				else
					s--;
			}
		}
	}
	else
	{
		s = pDraw->pPixels;

		// Unroll the first pass to boost DMA performance
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		if (iWidth <= TFT_BUFFER_SIZE)
			for (iCount = 0; iCount < iWidth; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];
		else
			for (iCount = 0; iCount < TFT_BUFFER_SIZE; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
		tft.dmaWait();
		tft.setAddrWindow(pDraw->iX + xoffset, y + yoffset, iWidth, 1);
		tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
		dmaBuf = !dmaBuf;
#else // 57.0 fps
		tft.setAddrWindow(pDraw->iX + xoffset, y + yoffset, iWidth, 1);
		tft.pushPixels(&usTemp[0][0], iCount);
#endif

		iWidth -= iCount;
		// Loop if pixel buffer smaller than width
		while (iWidth > 0)
		{
			// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
			if (iWidth <= TFT_BUFFER_SIZE)
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < TFT_BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA
			tft.dmaWait();
			tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
			dmaBuf = !dmaBuf;
#else
			tft.pushPixels(&usTemp[0][0], iCount);
#endif
			iWidth -= iCount;
		}
	}
}

static inline void displayGIF(void)
{
	tft.startWrite();

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Play GIF start");
	long runtime = micros();
	int frames = 0;
#endif

	while (gif.playFrame(true, NULL))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		frames++;
#endif
		yield();
	}
	gif.close();
	tft.endWrite();

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	runtime = micros() - runtime;
	Serial.print("Ran GIF "); Serial.print(" at "); Serial.print(frames / (runtime / 1000000.0)); Serial.println(" fps");
#endif
}

void showGIF(uint8_t *data, int size)
{
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(data, size, gifDrawLine))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Opened GIF "); Serial.print(" with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		xoffset = (TFT_DISPLAY_WIDTH - gif.getCanvasWidth()) / 2;
		yoffset = (TFT_DISPLAY_HEIGHT - gif.getCanvasHeight()) / 2;
		displayGIF();
	}
	else
	{
		Serial.println("Could not open GIF from RAM");
	}
}

void showGIF(const char *path)
{
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDrawLine))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Opened GIF "); Serial.print(path); Serial.print(" with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		xoffset = (TFT_DISPLAY_WIDTH - gif.getCanvasWidth()) / 2;
		yoffset = (TFT_DISPLAY_HEIGHT - gif.getCanvasHeight()) / 2;
		displayGIF();
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

static JPEGDEC jpeg;

int jpegDrawLine(JPEGDRAW *pDraw)
{
	tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
	return 1;
}

static inline void displayJPEG(void)
{
	xoffset = (TFT_DISPLAY_WIDTH - jpeg.getWidth()) / 2;
	yoffset = (TFT_DISPLAY_HEIGHT - jpeg.getHeight()) / 2;

	if (jpeg.getWidth() < TFT_DISPLAY_WIDTH || jpeg.getHeight() < TFT_DISPLAY_HEIGHT)
		tft.fillScreen(BACKGROUND_COLOR);

	tft.startWrite();
	jpeg.setPixelType(RGB565_BIG_ENDIAN);
	jpeg.decode(xoffset, yoffset, 0);
	jpeg.close();
	tft.endWrite();
}

void showJPEG(uint8_t *data, int size)
{
	if (jpeg.openRAM(data, size, jpegDrawLine))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened JPEG "); Serial.print(" with resolution "); Serial.print(jpeg.getWidth()); Serial.print(" x "); Serial.println(jpeg.getHeight());
#endif
		displayJPEG();
	}
	else
	{
		Serial.print("Could not open JPEG from RAM");
	}
}

void showJPEG(const char *path)
{
	if (jpeg.open(path, jpegOpen, jpegClose, jpegRead, jpegSeek, jpegDrawLine))
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened JPEG "); Serial.print(path); Serial.print(" with resolution "); Serial.print(jpeg.getWidth()); Serial.print(" x "); Serial.println(jpeg.getHeight());
#endif
		displayJPEG();
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

static PNG png;

void pngDrawLine(PNGDRAW *pDraw)
{
	uint16_t lineBuffer[MAX_IMAGE_WIDTH];
	png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, tft.color16to24(BACKGROUND_COLOR));
	tft.pushImage(xpos + xoffset, ypos + pDraw->y + yoffset, pDraw->iWidth, 1, lineBuffer);
}

static inline void displayPNG(void)
{
	xoffset = (TFT_DISPLAY_WIDTH - png.getWidth()) / 2;
	yoffset = (TFT_DISPLAY_HEIGHT - png.getHeight()) / 2;

	if (png.getWidth() < TFT_DISPLAY_WIDTH || png.getHeight() < TFT_DISPLAY_HEIGHT)
		tft.fillScreen(BACKGROUND_COLOR);

	tft.startWrite();
	png.decode(NULL, 0);
	tft.endWrite();
}

void showPNG(uint8_t *data, int size)
{
	if (png.openRAM(data, size, pngDrawLine) == PNG_SUCCESS)
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened PNG "); Serial.print(" with resolution "); Serial.print(png.getWidth()); Serial.print(" x "); Serial.println(png.getHeight());
#endif
		displayPNG();
	}
	else
	{
		Serial.print("Could not open PNG from RAM");
	}
}

void showPNG(const char *path)
{
	if (png.open(path, pngOpen, pngClose, pngRead, pngSeek, pngDrawLine) == PNG_SUCCESS)
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Opened PNG "); Serial.print(" with resolution "); Serial.print(png.getWidth()); Serial.print(" x "); Serial.println(png.getHeight());
#endif
		displayPNG();
	}
	else
	{
		Serial.print("Could not open PNG "); Serial.println(path);
	}
}

void showPNG(String path)
{
	showPNG(path.c_str());
}

/*******************************************************************************
 * Generic functions
 *******************************************************************************/

DisplayState getDisplayState(void)
{
	return displayState;
}

void setDisplayState(DisplayState state)
{
	displayState = state;
}

void showImage(const char *path)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Showing image: "); Serial.println(path);
#endif

	String pathString = String(path);
	pathString.trim();
	currentImage = pathString;

	if (currentImage.endsWith(".jpg") || currentImage.endsWith(".JPG") || currentImage.endsWith(".jpeg") || currentImage.endsWith(".JPEG"))
	{
		displayState = DISPLAY_STATIC_IMAGE;
		showJPEG(path);
	}
	else if (currentImage.endsWith(".png") || currentImage.endsWith(".PNG"))
	{
		displayState = DISPLAY_STATIC_IMAGE;
		showPNG(path);
	}
	else if (currentImage.endsWith(".gif") || currentImage.endsWith(".GIF"))
	{
		displayState = DISPLAY_ANIMATED_GIF;
		showGIF(path);
	}
}

void showImage(String path)
{
	showImage(path.c_str());
}

void showMister(void)
{
	displayState = DISPLAY_MISTER;
	showGIF((uint8_t *)mister, sizeof(mister));
}

void showStartup(void)
{
#if defined(STARTUP_LOGO)
  showImage(STARTUP_LOGO);
#else
  showMister();
#endif
}

/*******************************************************************************
 * Slideshow functions
 *******************************************************************************/

static void runSlideshow(long time)
{
	static long nextChange = 0;

	if (time < nextChange)
		return;

	String nextFile = getNextFile();
	if (nextFile == "")
	{
		const char *dirName = getDirectory().c_str();
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("No slideshow file found, reset directory for: "); Serial.println(dirName);
#endif
		rewindDirectory();
	}
	else
	{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		Serial.print("Found slideshow file: "); Serial.println(nextFile.c_str());
#endif
		showImage(nextFile);
		displayState = DISPLAY_SLIDESHOW; // Need to explicitly set state here since showImage methods will update it
		nextChange = time + SLIDESHOW_DELAY;
	}
}

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

void loopDisplay(long time)
{
	switch (displayState)
	{
		case DISPLAY_ANIMATED_GIF: showGIF(currentImage);   break;
		case DISPLAY_SLIDESHOW:    runSlideshow(time);      break;
		case DISPLAY_MISTER:       showMister();            break;
		default:                                            break;
	}
}

#endif
