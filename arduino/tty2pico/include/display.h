#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "storage.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include <PNGdec.h>
#include "mister_kun_blink.h"

#define DISABLE_COLOR_MIXING 0xffffffff

typedef enum DisplayState {
	DISPLAY_ANIMATED_GIF,
	DISPLAY_ANIMATED_GIF_LOOPING,
	DISPLAY_MISTER,
	DISPLAY_RANDOM_SHAPES,
	DISPLAY_SLIDESHOW,
	DISPLAY_STATIC_IMAGE,
	DISPLAY_STATIC_TEXT,
} DisplayState;

typedef enum Fonts {
	// GFXFF =  FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
	GLCD = 1, // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
	FONT2 = 2, // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
	FONT4 = 4, // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
	FONT6 = 6, // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
	FONT7 = 7, // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
	FONT8 = 8, // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
} Fonts;

#define FONT_SMALL (TFT_DISPLAY_HEIGHT < 160 ? GLCD : FONT2)
#define FONT_LARGE (TFT_DISPLAY_HEIGHT < 160 ? FONT2 : FONT4)
#define FONT_SMALL_SIZE (TFT_DISPLAY_HEIGHT < 160 ? 8 : 16)
#define FONT_LARGE_SIZE (TFT_DISPLAY_HEIGHT < 160 ? 16 : 26)
#define DISPLAY_TEXT_MARGIN 4

const char *imageExtensions[] = {
	".gif",
	".png",
};
const int imageExtensionCount = sizeof(imageExtensions) / sizeof(imageExtensions[0]);

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
TFT_eSprite displayBuffer(&tft);

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

inline void clearDisplay(void)
{
	tft.fillScreen(BACKGROUND_COLOR);
}

/*******************************************************************************
 * Text and Graphics functions
 *******************************************************************************/

#define DEMO_SQUARE_SIZE (TFT_DISPLAY_WIDTH / 3)
#define DEMO_CIRCLE_RADIUS (DEMO_SQUARE_SIZE / 2)
#define DEMO_ELLIPSE_RADIUS_X (DEMO_SQUARE_SIZE / 3)
#define DEMO_ELLIPSE_RADIUS_Y (DEMO_SQUARE_SIZE / 2)
#define DEMO_TRIANGLE_SIZE (DEMO_SQUARE_SIZE / 2)

void drawDemoShapes(int durationMS)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Demo shapes start");
	long runtime = micros();
	int frames = 0;
#endif
	displayState = DISPLAY_RANDOM_SHAPES;

	long startTime = millis();
	long endTime = (durationMS <= 0) ? LONG_MAX : (startTime + durationMS);

	int squareX = TFT_DISPLAY_WIDTH / 2;
	const int minFrameCount = -(DEMO_SQUARE_SIZE / 2);
	const int maxFrameCount = TFT_DISPLAY_WIDTH + (DEMO_SQUARE_SIZE / 2);
	int frame = -DEMO_SQUARE_SIZE;
	bool reverse = false;

	displayBuffer.createSprite(TFT_DISPLAY_WIDTH, TFT_DISPLAY_HEIGHT);
	while (millis() < endTime)
	{
		displayBuffer.fillScreen(TFT_BLACK);
		displayBuffer.fillCircle(frame, TFT_DISPLAY_WIDTH - frame, DEMO_CIRCLE_RADIUS, TFT_RED);
		displayBuffer.fillRoundRect(TFT_DISPLAY_WIDTH - (DEMO_SQUARE_SIZE / 2) - frame, frame - (DEMO_SQUARE_SIZE / 2), DEMO_SQUARE_SIZE, DEMO_SQUARE_SIZE, 3, TFT_GREEN);
		displayBuffer.fillTriangle(frame, frame - DEMO_TRIANGLE_SIZE, frame - DEMO_TRIANGLE_SIZE, frame + DEMO_TRIANGLE_SIZE, frame + DEMO_TRIANGLE_SIZE, frame + DEMO_TRIANGLE_SIZE, TFT_BLUE);
		displayBuffer.fillEllipse(TFT_DISPLAY_WIDTH - frame, TFT_DISPLAY_WIDTH - frame, DEMO_ELLIPSE_RADIUS_X, DEMO_ELLIPSE_RADIUS_Y, TFT_YELLOW);
		displayBuffer.pushSprite(0, 0);

		if (reverse)
			frame--;
		else
			frame++;

		if (frame == maxFrameCount)
			reverse = true;
		else if (frame == minFrameCount)
			reverse = false;

#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
		frames++;
#endif
	}
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	runtime = micros() - runtime;
	Serial.print("Demo random shapes at "); Serial.print(frames / (runtime / 1000000.0)); Serial.println(" fps");
#endif
	displayBuffer.deleteSprite();
}

void showText(const char *displayText, uint8_t font = FONT_LARGE, uint16_t textColor = TFT_WHITE, uint16_t backgroundColor = TFT_BLACK)
{
	displayState = DISPLAY_STATIC_TEXT;
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.print("Showing message: "); Serial.println(displayText);
#endif
	displayBuffer.createSprite(TFT_DISPLAY_WIDTH, TFT_DISPLAY_HEIGHT);
	displayBuffer.fillSprite(backgroundColor);
	displayBuffer.setTextColor(textColor);
	displayBuffer.drawCentreString(displayText, TFT_DISPLAY_WIDTH / 2, TFT_DISPLAY_HEIGHT / 2, font);
	displayBuffer.pushSprite(0, 0);
	displayBuffer.deleteSprite();
}

void showHeaderedText(String lines[], int lineCount)
{
	displayState = DISPLAY_STATIC_TEXT;
	displayBuffer.createSprite(TFT_DISPLAY_WIDTH, TFT_DISPLAY_HEIGHT);
	displayBuffer.fillSprite(TFT_BLACK);
	displayBuffer.setTextColor(TFT_WHITE);

	int start = -(lineCount / 2);
	int yOffset = 0;
	for (int i = 0; i < lineCount; i++)
	{
		yOffset = ((i == 0) ? FONT_LARGE_SIZE : 0) + (-(i + start + 1) * FONT_SMALL_SIZE) + (3 * DISPLAY_TEXT_MARGIN);
		displayBuffer.drawCentreString(lines[i], TFT_MIDPOINT_X, TFT_MIDPOINT_Y - yOffset, (i == 0) ? FONT_LARGE : FONT_SMALL);
	}

	displayBuffer.pushSprite(0, 0);
	displayBuffer.deleteSprite();
}

void showSystemInfo(void)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Showing system info");
#endif

	String lines[] =
	{
		String("tty2pico"),
		"v" + String(TTY2PICO_VERSION),
		String(TTY2PICO_BOARD),
		String((clock_get_hz(clk_sys) / 1000000.0f)) + "MHz @ " + analogReadTemp() + "C",
		String(TFT_DISPLAY_WIDTH) + "x" + String(TFT_DISPLAY_HEIGHT) + " " + String(TTY2PICO_DISPLAY),
		"SPI @ " + String(SPI_FREQUENCY / 1000000.0f) + "MHz",
		getHasSD() ? "SD Filesystem" : "Flash Filesystem",
	};

	showHeaderedText(lines, 7);
}

/*******************************************************************************
 * GIF
 *******************************************************************************/

#define TFT_BUFFER_SIZE TFT_DISPLAY_WIDTH

#ifdef USE_DMA
static uint16_t usTemp[2][TFT_BUFFER_SIZE]; // Global display buffer for DMA use
#else
static uint16_t usTemp[1][TFT_BUFFER_SIZE]; // Global display buffer
#endif
static bool dmaBuf = 0;

#ifdef USE_GIF_BUFFERING
static void gifDrawLine(GIFDRAW *pDraw)
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
		uint8_t *pEnd, color, ucTransparent = pDraw->ucTransparent;
		pEnd = s + iWidth;
		x = 0;
		iCount = 0; // count non-transparent pixels
		while (x < iWidth)
		{
			color = ucTransparent - 1;
			d = &usTemp[0][0];
			while (color != ucTransparent && s < pEnd && iCount < TFT_BUFFER_SIZE)
			{
				color = *s++;
				if (color == ucTransparent) // done, stop
				{
					s--; // back up to treat it like transparent
				}
				else // opaque
				{
					*d++ = usPalette[color];
					iCount++;
				}
			} // while looking for opaque pixels
			if (iCount) // any opaque pixels?
			{
				displayBuffer.pushImage(pDraw->iX + x + xoffset, y + yoffset, iCount, 1, &usTemp[0][0]);
				x += iCount;
				iCount = 0;
			}
			// no, look for a run of transparent pixels
			color = ucTransparent;
			while (color == ucTransparent && s < pEnd)
			{
				color = *s++;
				if (color == ucTransparent)
					x++;
				else
					s--;
			}
		}
	}
	else
	{
		s = pDraw->pPixels;
		displayBuffer.pushImage(pDraw->iX + xoffset, y + yoffset, iWidth, 1, &usTemp[0][0]);

		iWidth -= iCount;
		// Loop if pixel buffer smaller than width
		while (iWidth > 0)
		{
			// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
			if (iWidth <= TFT_BUFFER_SIZE)
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < TFT_BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

			displayBuffer.pushImage(pDraw->iX + iCount + xoffset, y + yoffset, iCount, 1, &usTemp[0][0]);
			iWidth -= iCount;
		}
	}
}

static inline void displayGIF(AnimatedGIF *gif, bool loop = false)
{
	int width = gif->getCanvasWidth();
	int height = gif->getCanvasHeight();
	xoffset = (TFT_DISPLAY_WIDTH - width) / 2;
	yoffset = (TFT_DISPLAY_HEIGHT - height) / 2;
	displayState = loop ? DISPLAY_ANIMATED_GIF_LOOPING : DISPLAY_ANIMATED_GIF;

	displayBuffer.createSprite(TFT_DISPLAY_WIDTH, TFT_DISPLAY_HEIGHT);

#if VERBOSE_OUTPUT == 1
	Serial.println("Play GIF start");
	long runtime = micros();
	int frames = 0;
#endif

	while (gif->playFrame(true, NULL))
	{
#if VERBOSE_OUTPUT == 1
		frames++;
#endif
		displayBuffer.pushSprite(0, 0);
		// yield();
	}

	if (gif->getLastError() == GIF_SUCCESS)
	{
#if VERBOSE_OUTPUT == 1
		frames++;
#endif
		displayBuffer.pushSprite(0, 0);
	}

#if VERBOSE_OUTPUT == 1
	runtime = micros() - runtime;
	Serial.print("Ran bufferred GIF "); Serial.print(" at "); Serial.print(frames / (runtime / 1000000.0)); Serial.println(" fps");
#endif

	displayBuffer.deleteSprite();
}
#else
// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
// Draw a line of image directly on the LCD
static void gifDrawLine(GIFDRAW *pDraw)
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

#ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
		// Unroll the first pass to boost DMA performance
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		if (iWidth <= TFT_BUFFER_SIZE)
			for (iCount = 0; iCount < iWidth; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];
		else
			for (iCount = 0; iCount < TFT_BUFFER_SIZE; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];

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

static inline void displayGIF(AnimatedGIF *gif, bool loop = false)
{
	xoffset = (TFT_DISPLAY_WIDTH - gif->getCanvasWidth()) / 2;
	yoffset = (TFT_DISPLAY_HEIGHT - gif->getCanvasHeight()) / 2;
	displayState = loop ? DISPLAY_ANIMATED_GIF_LOOPING : DISPLAY_ANIMATED_GIF;
	tft.startWrite();

#if VERBOSE_OUTPUT == 1
	Serial.println("Play GIF start");
	long runtime = micros();
	int frames = 0;
#endif

	while (gif->playFrame(true, NULL))
	{
#if VERBOSE_OUTPUT == 1
		frames++;
#endif
		yield();
	}
	tft.endWrite();

#if VERBOSE_OUTPUT == 1
	runtime = micros() - runtime;
	Serial.print("Ran GIF "); Serial.print(" at "); Serial.print(frames / (runtime / 1000000.0)); Serial.println(" fps");
#endif
}
#endif

static void showGIF(uint8_t *data, int size, bool loop = false)
{
	AnimatedGIF gif;
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(data, size, gifDrawLine))
	{
#if VERBOSE_OUTPUT == 1
	Serial.print("Opened streamed GIF with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		displayGIF(&gif, loop);
		gif.close();
	}
	else
	{
		Serial.println("Could not open GIF from RAM");
	}
}

static void showGIF(const char *path, bool loop = false)
{
	AnimatedGIF gif;
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDrawLine))
	{
#if VERBOSE_OUTPUT == 1
	Serial.print("Opened GIF "); Serial.print(path); Serial.print(" with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		displayGIF(&gif, loop);
		gif.close();
	}
	else
	{
		Serial.print("Could not open GIF "); Serial.println(path);
	}
}

static void showGIF(String path, bool loop = false)
{
	showGIF(path.c_str(), loop);
}

/*******************************************************************************
 * PNG
 *******************************************************************************/

static void pngDrawLine(PNGDRAW *pDraw)
{
	uint16_t lineBuffer[MAX_IMAGE_WIDTH];
	PNG *png = ((PNG *)pDraw->pUser);
	bool hasAlpha = png->hasAlpha();
	png->getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, tft.color16to24(BACKGROUND_COLOR));
	displayBuffer.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

static inline void displayPNG(PNG &png)
{
	int width = png.getWidth();
	int height = png.getHeight();
	xoffset = (TFT_DISPLAY_WIDTH - width) / 2;
	yoffset = (TFT_DISPLAY_HEIGHT - height) / 2;
	displayState = DISPLAY_STATIC_IMAGE;

	displayBuffer.createSprite(width, height);
	displayBuffer.fillSprite(TFT_TRANSPARENT);

	png.decode(NULL, 0); // Fill displayBuffer sprite with image data

	if (width < TFT_DISPLAY_WIDTH || height < TFT_DISPLAY_HEIGHT || !png.hasAlpha())
	{
		// When PNG is smaller than display and not transparent use the top-left pixel of the image for background
		uint32_t topLeftColor = displayBuffer.readPixel(0, 0);
#if VERBOSE_OUTPUT == 1
		Serial.print("Filling screen with "); Serial.println(topLeftColor, HEX);
#endif
		tft.fillScreen(topLeftColor);
	}
	else clearDisplay();

	displayBuffer.pushSprite(xoffset, yoffset, TFT_TRANSPARENT);
	displayBuffer.deleteSprite();
}

static void showPNG(uint8_t *data, int size)
{
	PNG png;
	if (png.openRAM(data, size, pngDrawLine) == PNG_SUCCESS)
	{
#if VERBOSE_OUTPUT == 1
		Serial.print("Opened streamed PNG with resolution "); Serial.print(png.getWidth()); Serial.print("x"); Serial.println(png.getHeight());
#endif
		displayPNG(png);
		png.close();
	}
	else
	{
		Serial.print("Could not open PNG from RAM");
	}
}

static void showPNG(const char *path)
{
	PNG png;
	if (png.open(path, pngOpen, pngClose, pngRead, pngSeek, pngDrawLine) == PNG_SUCCESS)
	{
#if VERBOSE_OUTPUT == 1
		Serial.print("Opened PNG "); Serial.print(path); Serial.print(" with resolution "); Serial.print(png.getWidth()); Serial.print("x"); Serial.println(png.getHeight());
#endif
		displayPNG(png);
		png.close();
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
#if VERBOSE_OUTPUT == 1
	Serial.print("Showing image: "); Serial.println(path);
#endif
	if (fileExists(path))
	{
		String pathString = String(path);
		pathString.trim();
		pathString.toLowerCase();
		currentImage = pathString;

		if (currentImage.endsWith(".loop.gif"))
			showGIF(path, true);
		else if (currentImage.endsWith(".gif"))
			showGIF(path);
		else if (currentImage.endsWith(".png"))
			showPNG(path);
		else
			showText(path);
	}
	else showText(path);
}

void showImage(String path)
{
	showImage(path.c_str());
}

void showMister(void)
{
	showGIF((uint8_t *)mister_kun_blink, sizeof(mister_kun_blink));
	displayState = DISPLAY_MISTER; // Explicitly set display state since the displayGIF() method set it
}

void showStartup(void)
{
	showSystemInfo();
	delay(3000);

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
#if VERBOSE_OUTPUT == 1
		Serial.print("No slideshow file found, reset directory for: "); Serial.println(dirName);
#endif
		rewindDirectory();
	}
	else
	{
#if VERBOSE_OUTPUT == 1
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
		case DISPLAY_ANIMATED_GIF_LOOPING: showGIF(currentImage, true);   break;
		case DISPLAY_SLIDESHOW:            runSlideshow(time);            break;
		case DISPLAY_MISTER:               showMister();                  break;
		default:                                                          break;
	}
}

#endif
