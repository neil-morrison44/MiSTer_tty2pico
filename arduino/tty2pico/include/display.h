#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "definitions.h"
#include "storage.h"
#include "platform.h"
#include <vector>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include <PNGdec.h>
#include "mister_kun_blink.h"

using namespace std;

#define DISABLE_COLOR_MIXING 0xffffffff

#define DISPLAY_TEXT_MARGIN 4

const char *imageExtensions[] = {
	".loop.fast.gif",
	".loop.gif",
	".fast.gif",
	".gif",
	".png",
};
const int imageExtensionCount = sizeof(imageExtensions) / sizeof(imageExtensions[0]);

static int32_t xoffset = 0;
static int32_t yoffset = 0;
static String currentImage;
static DisplayState displayState = DISPLAY_STATIC_IMAGE;
#if SHOW_FPS == 1
static float fps;
#endif

/*******************************************************************************
 * Display setup
 *******************************************************************************/

TFT_eSPI tft = TFT_eSPI(config.tftWidth, config.tftHeight);
TFT_eSprite displayBuffer(&tft);
const int displayBufferCount = config.getDisplayWidth() * config.getDisplayHeight();

void setupDisplay()
{
	Serial.println("Setting up display...");

#if defined(TFT_BL)
	// Turn off backlight before starting up screen
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, LOW);
#endif

	// Setup screen
	tft.init();
#if USE_DMA == 1
	tft.initDMA();
#endif
	tft.setRotation(config.tftRotation);
	tft.setTextFont(config.getFontSmall());
	tft.setTextWrap(true);

	// Clear display
	tft.fillScreen(TFT_BLACK);

	// Open exclusive use of the SPI channel for the display
	tft.startWrite();

#if defined(TFT_BL)
	delay(50); // Small delay to avoid garbage output
	digitalWrite(TFT_BL, HIGH); // Turn backlight back on after init
#endif

	Serial.println("Display setup complete");
}

/*******************************************************************************
 * Text and Graphics functions
 *******************************************************************************/

inline void __attribute__((always_inline)) clearBuffer(TFT_eSprite *buffer = &displayBuffer)
{
#if USE_DMA == 1
	tft.dmaWait();
#endif
	buffer->deleteSprite();
}

inline void __attribute__((always_inline)) clearDisplay(void)
{
	tft.fillScreen(config.backgroundColor);
}

// Draw various shapes on the screen and animate them for the specified duration
void drawDemoShapes(int durationMS)
{
#if defined(VERBOSE_OUTPUT) && VERBOSE_OUTPUT == 1
	Serial.println("Demo shapes start");
	long runtime = micros();
	int frames = 0;
#endif
	int squareSize = config.tftWidth / 3;
	int circleRadius = squareSize / 2;
	int ellipseRadiusX = squareSize / 3;
	int ellipseRadiusY = squareSize / 2;
	int triangleSize = squareSize / 2;

	displayState = DISPLAY_RANDOM_SHAPES;

	long startTime = millis();
	long endTime = (durationMS <= 0) ? LONG_MAX : (startTime + durationMS);

	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();
	int squareX = displayWidth / 2;
	const int minFrameCount = -(squareSize / 2);
	const int maxFrameCount = displayWidth + (squareSize / 2);
	int frame = -squareSize;
	bool reverse = false;

	clearBuffer();
	uint16_t *pixelBuf = (uint16_t *)displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());

	while (millis() < endTime)
	{
		displayBuffer.fillScreen(TFT_BLACK);
		displayBuffer.fillCircle(frame, displayWidth - frame, circleRadius, TFT_RED);
		displayBuffer.fillRoundRect(displayWidth - (squareSize / 2) - frame, frame - (squareSize / 2), squareSize, squareSize, 3, TFT_GREEN);
		displayBuffer.fillTriangle(frame, frame - triangleSize, frame - triangleSize, frame + triangleSize, frame + triangleSize, frame + triangleSize, TFT_BLUE);
		displayBuffer.fillEllipse(displayWidth - frame, displayWidth - frame, ellipseRadiusX, ellipseRadiusY, TFT_YELLOW);
#if USE_DMA == 1
		tft.dmaWait();
		tft.pushImageDMA(0, 0, displayWidth, displayHeight, pixelBuf);
#else
		displayBuffer.pushSprite(0, 0);
#endif

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
	Serial.print("Demo random shapes at "); Serial.print(frames / (runtime / 1000000.0f)); Serial.println(" fps");
#endif
	clearBuffer();
}

// Draws a transparent overlay on top of whatever is in the display buffer
void overlayText(String text, TFT_eSPI *parent)
{
	int font = config.getFontSmall();
	TFT_eSprite overlay(parent);
	int width = tft.textWidth(text, font) + (DISPLAY_TEXT_MARGIN * 2);
	int height = tft.fontHeight(font) + (DISPLAY_TEXT_MARGIN * 2);
	uint16_t *pixelBuf = (uint16_t *)overlay.createSprite(width, height);
	overlay.fillSprite(TFT_TRANSPARENT);
	overlay.setTextColor(TFT_WHITE);
	overlay.setTextFont(font);
	overlay.drawString(text, DISPLAY_TEXT_MARGIN, DISPLAY_TEXT_MARGIN, font);
#if USE_DMA == 1
	tft.dmaWait();
	tft.pushImageDMA((config.getDisplayWidth() - width) / 2, (config.getDisplayHeight() - height) / 2, width, height, pixelBuf);
#else
	overlay.pushSprite((config.getDisplayWidth() - width) / 2, (config.getDisplayHeight() - height) / 2);
#endif
	clearBuffer(&overlay);
}

void overlayText(String text)
{
	overlayText(text, &tft);
}

#if SHOW_FPS == 1
// Show the last captured FPS value
void showFPS(TFT_eSPI *parent)
{
	overlayText(String(fps), parent);
}

// Show the last captured FPS value
void showFPS(void)
{
	overlayText(String(fps), &tft);
}

// Show FPS by providing a new value
void showFPS(float newFPS, TFT_eSPI *parent)
{
	fps = newFPS;
	overlayText(String(fps), parent);
}

// Show FPS by providing a new value
void showFPS(float newFPS)
{
	fps = newFPS;
	overlayText(String(fps), &tft);
}

// Show FPS by providing the during in microseconds of the last frame
void showFPS(unsigned long micros, TFT_eSPI *parent)
{
	overlayText(String(1000000.0f / micros), parent);
}

// Show FPS by providing the during in microseconds of the last frame
void showFPS(unsigned long micros)
{
	overlayText(String(1000000.0f / micros), &tft);
}
#endif

// Draw lines of text on the screen with center alignment horizontally and veritcally
void showText(String lines[], int lineCount, uint8_t font, uint16_t textColor = TFT_WHITE, uint16_t backgroundColor = TFT_BLACK)
{
	displayState = DISPLAY_STATIC_TEXT;

	clearBuffer();

	uint16_t *pixelBuf = (uint16_t *)displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());
	displayBuffer.fillSprite(backgroundColor);
	displayBuffer.setTextColor(textColor);
	displayBuffer.setTextFont(font);

	int midpointX = config.getMidpointX();
	int textWidth = displayBuffer.textWidth(" ", font);
	int textHeight = displayBuffer.fontHeight(font);
	int lineHeight = textHeight + DISPLAY_TEXT_MARGIN;
	int yStart = config.getMidpointY() - (lineCount / 2.0 * lineHeight);

	for (int i = 0; i < lineCount; i++)
		displayBuffer.drawCentreString(lines[i], midpointX, yStart + (i * lineHeight), font);

#if USE_DMA == 1
	tft.dmaWait();
	tft.pushImageDMA(0, 0, config.getDisplayWidth(), config.getDisplayHeight(), pixelBuf);
#else
	displayBuffer.pushSprite(0, 0);
#endif
	clearBuffer();
}

// Draw lines of text on the screen with center alignment horizontally and veritcally
void showText(String lines[], int lineCount, uint16_t textColor = TFT_WHITE, uint16_t backgroundColor = TFT_BLACK)
{
	showText(lines, lineCount, config.getFontLarge(), textColor, backgroundColor);
}

// Draw a string of text on the screen with automatic word breaks, word wrap and center alignment horizontally and veritcally
void showText(String text, uint8_t font, uint16_t textColor = TFT_WHITE, uint16_t backgroundColor = TFT_BLACK)
{
	// Get font properties
	int fontHeight = tft.textWidth(" ", font);
	int fontWidth = tft.fontHeight(font);

	// Calculate text area parameters
	int maxLineWidth = (config.getDisplayWidth() / 5) * 4; // Use 1/5 margin
	int startX = (config.getDisplayWidth() - maxLineWidth) / 2;
	int charCount = text.length();
	int charsPerLine = maxLineWidth / fontWidth;

	// Split words on spaces
	int lineIndex = 0;
	int startWord = 0, endWord = 0;
	vector<String> words;
	words.push_back("");

	for (int c = 0; c < charCount; c++)
	{
		char cc = text.charAt(c);
		if (cc == ' ' && words[lineIndex].length() > 0)
		{
			words.push_back("");
			lineIndex++;
		}
		else words[lineIndex] += cc;
	}

	// Align words to lines
	const int spaceWidth = tft.textWidth(" ", font);
	int charWidth = 0, wordWidth = 0, lineWidth = 0, wordCharCount = 0;
	lineIndex = 0;
	vector<String> lines;
	lines.push_back("");

	// Iterate each word
	for (String word : words)
	{
		// Compute word width
		wordWidth = 0;
		wordCharCount = word.length();
		for (int wc = 0; wc < wordCharCount; wc++)
			wordWidth += tft.textWidth(String(word.charAt(wc)), font);

		// Will word (with space prefix) fit on line?
		if ((lineWidth + spaceWidth + wordWidth) < maxLineWidth)
		{
			// Append word (with space) to current line
			lines[lineIndex] += " " + word;
			lineWidth += (spaceWidth + wordWidth);
		}
		else
		{
			// Will word (without space prefix) fit on single full line?
			if (wordWidth < maxLineWidth)
			{
				// Append word as new line
				lines.push_back(word);
				lineWidth = wordWidth;
				lineIndex++;
			}
			else
			{
				// Take chars from beginning of temp string (plus '-') to max line width
				int dashWidth = tft.textWidth("-", font);
				lines.push_back("");
				lineWidth = dashWidth;
				wordCharCount = word.length();

				for (int i = 0; i < wordCharCount; i++)
				{
					String wc = String(word.charAt(i));
					charWidth = tft.textWidth(wc, font);

					// Will temp char fit on the current line?
					if ((charWidth + lineWidth) < maxLineWidth)
					{
						// Append char to current line
						lines[lineIndex] += wc;
						lineWidth += charWidth;
					}
					else
					{
						// Finish current line and append char to new line
						lines[lineIndex] += "-";
						lines.push_back(wc);
						lineIndex++;
						lineWidth = dashWidth + charWidth;
					}
				}
			}
		}
	}

	showText(lines.data(), lineIndex + 1, font, textColor, backgroundColor);
}

// Draw a string of text on the screen with automatic word breaks, word wrap and center alignment horizontally and veritcally
void showText(String line, uint16_t textColor = TFT_WHITE, uint16_t backgroundColor = TFT_BLACK)
{
	showText(line, config.getFontLarge(), textColor, backgroundColor);
}

// Draw "message box" style text with the first line being the header, and center alignment horizontally and veritcally for all text
void showHeaderedText(String lines[], int lineCount)
{
	displayState = DISPLAY_STATIC_TEXT;

	clearBuffer();

	uint16_t *pixelBuf = (uint16_t *)displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());
	displayBuffer.fillSprite(TFT_BLACK);
	displayBuffer.setTextColor(TFT_WHITE);

	int midpointX = config.getMidpointX();
	int midpointY = config.getMidpointY();
	int start = -(lineCount / 2) - (lineCount % 2);
	int yOffset = 0;

	for (int i = 0; i < lineCount; i++)
	{
		yOffset = ((i == 0) ? config.getFontLargeSize() : 0) + (-(i + start + 1) * config.getFontSmallSize()) + DISPLAY_TEXT_MARGIN;
		displayBuffer.drawCentreString(lines[i], midpointX, midpointY - yOffset, (i == 0) ? config.getFontLarge() : config.getFontSmall());
	}

#if USE_DMA == 1
	tft.dmaWait();
	tft.pushImageDMA(0, 0, config.getDisplayWidth(), config.getDisplayHeight(), pixelBuf);
#else
	displayBuffer.pushSprite(0, 0);
#endif
	clearBuffer();
}

// Display a frame of the system information screen
void showSystemInfo(uint32_t frameTime)
{
	static uint32_t nextFrameTime;

	int32_t frameTimeDiff = frameTime - nextFrameTime;
	if (frameTimeDiff < 0)
		return;

	displayState = DISPLAY_SYSTEM_INFORMATION;

	vector<String> lines;
	char lineBuffer[50];
	lines.push_back("tty2pico");
	lines.push_back("v" + String(TTY2PICO_VERSION_STRING));
	lines.push_back(TTY2PICO_BOARD);

	snprintf(lineBuffer, sizeof(lineBuffer), "RP2040 @ %.1fMHz %.1fC", getCpuSpeedMHz(), getCpuTemperature());
	lines.push_back(lineBuffer);

	memset(lineBuffer, 0, sizeof(lineBuffer));
	float displaySpi = getSpiRateDisplayMHz();
	snprintf(lineBuffer, sizeof(lineBuffer), "%s @ %dx%d %.1fMHz", TTY2PICO_DISPLAY, config.getDisplayWidth(), config.getDisplayHeight(), displaySpi);
	lines.push_back(lineBuffer);

	lines.push_back(String(getTime(DTF_HUMAN)));

	if (getHasSD())
	{
		memset(lineBuffer, 0, sizeof(lineBuffer));
		snprintf(lineBuffer, sizeof(lineBuffer), "SD Filesystem @ %.1fMHz", getSpiRateSdMHz());
		lines.push_back(lineBuffer);
	}
	else lines.push_back("Flash Filesystem");

	showHeaderedText(lines.data(), lines.size());

	nextFrameTime = frameTime + 1000; // Only update display once every second
}

/*******************************************************************************
 * GIF
 *******************************************************************************/

struct GIFDisplayOptions
{
	bool loop;
	bool uncap;
};
const GIFDisplayOptions GIF_ONCE      = { .loop = false, .uncap = false };
const GIFDisplayOptions GIF_FAST      = { .loop = false, .uncap = true };
const GIFDisplayOptions GIF_LOOP      = { .loop = true,  .uncap = false };
const GIFDisplayOptions GIF_LOOP_FAST = { .loop = true,  .uncap = true };
static GIFDisplayOptions currentGifOption;

// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
// Draw a line of image directly on the LCD
[[deprecated("Replaced with gifDrawBufferedLine")]]
static void gifDrawLine(GIFDRAW *pDraw)
{
#if USE_DMA == 1
	static uint16_t usTemp[2][TFT_DISPLAY_MAX];
#else
	static uint16_t usTemp[1][TFT_DISPLAY_MAX];
#endif

	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();

	bool dmaBuf = 0;

	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, y, iWidth, iCount;

	// Display bounds check and cropping
	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > displayWidth)
		iWidth = displayWidth - pDraw->iX;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line
	if (y >= displayHeight || pDraw->iX >= displayWidth || iWidth < 1)
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
			d = &usTemp[dmaBuf][0];
			while (c != ucTransparent && s < pEnd && iCount < TFT_DISPLAY_MAX)
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
				tft.setAddrWindow(pDraw->iX + x + xoffset, y + yoffset, iCount, 1);
				tft.pushPixels(&usTemp[dmaBuf][0], iCount);
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

		tft.setAddrWindow(pDraw->iX + xoffset, y + yoffset, iWidth, 1);

#if USE_DMA == 1
		// Unroll the first pass to boost DMA performance
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		if (iWidth <= TFT_DISPLAY_MAX)
			for (iCount = 0; iCount < iWidth; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];
		else
			for (iCount = 0; iCount < TFT_DISPLAY_MAX; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];
#endif

#if USE_DMA == 1
		tft.dmaWait();
		tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
		dmaBuf = !dmaBuf;
#else
		tft.pushPixels(&usTemp[dmaBuf][0], iCount);
#endif

		iWidth -= iCount;
		// Loop if pixel buffer smaller than width
		while (iWidth > 0)
		{
			// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
			if (iWidth <= TFT_DISPLAY_MAX)
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < TFT_DISPLAY_MAX; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#if USE_DMA == 1
			tft.dmaWait();
			tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
			dmaBuf = !dmaBuf;
#else
			tft.pushPixels(&usTemp[dmaBuf][0], iCount);
#endif

			iWidth -= iCount;
		}
	}
}

static void gifDrawBufferedLine(GIFDRAW *pDraw)
{
	static uint16_t usTemp[TFT_DISPLAY_MAX];

	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();

	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, y, iWidth, iCount;

	// Display bounds check and cropping
	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > displayWidth)
		iWidth = displayWidth - pDraw->iX;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line
	if (y >= displayHeight || pDraw->iX >= displayWidth || iWidth < 1)
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
			d = &usTemp[0];
			while (c != ucTransparent && s < pEnd && iCount < TFT_DISPLAY_MAX)
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
				displayBuffer.pushImage(pDraw->iX + x + xoffset, y + yoffset, iCount, 1, &usTemp[0]);
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

#if USE_DMA == 1
		// Unroll the first pass to boost DMA performance
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		if (iWidth <= TFT_DISPLAY_MAX)
			for (iCount = 0; iCount < iWidth; iCount++)
				usTemp[iCount] = usPalette[*s++];
		else
			for (iCount = 0; iCount < TFT_DISPLAY_MAX; iCount++)
				usTemp[iCount] = usPalette[*s++];
#endif

		displayBuffer.pushImage(pDraw->iX + xoffset, y + yoffset, iWidth, 1, &usTemp[0]);

		iWidth -= iCount;
		// Loop if pixel buffer smaller than width
		while (iWidth > 0)
		{
			// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
			if (iWidth <= TFT_DISPLAY_MAX)
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < TFT_DISPLAY_MAX; iCount++) usTemp[iCount] = usPalette[*s++];

			displayBuffer.pushImage(pDraw->iX + xoffset, y + yoffset, iWidth, 1, &usTemp[0]);

			iWidth -= iCount;
		}
	}

	if (y == pDraw->iHeight - 1)
	{
#if USE_DMA == 1
		tft.dmaWait();
		tft.pushPixelsDMA((uint16_t *)pDraw->pUser, displayBufferCount);
#else
		displayBuffer.pushSprite(0, 0);
#endif
	}
}

static inline void displayGIF(AnimatedGIF *gif, GIFDisplayOptions options)
{
	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();
	xoffset = (displayWidth - gif->getCanvasWidth()) / 2;
	yoffset = (displayHeight - gif->getCanvasHeight()) / 2;
	displayState = options.loop ? DISPLAY_ANIMATED_GIF_LOOPING : DISPLAY_ANIMATED_GIF;

	clearBuffer();
	uint16_t *pixelBuf = (uint16_t *)displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());
	displayBuffer.fillSprite(config.backgroundColor);

#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
	int frames = 0;
	unsigned long runStart = micros();
#endif
#if SHOW_FPS == 1
	unsigned long frameStart = micros();
#endif

	while (gif->playFrame(!(options.uncap || config.uncapFramerate), NULL, pixelBuf))
	{
#if SHOW_FPS == 1
		showFPS(micros() - frameStart);
#endif
#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
		frames++;
#endif
		yield();
#if SHOW_FPS == 1
		frameStart = micros();
#endif
	}

#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
	unsigned long runStop = micros();
#endif
	if (gif->getLastError() == GIF_SUCCESS)
	{
#if SHOW_FPS == 1
		showFPS(runStop - frameStart);
#endif
#if VERBOSE_OUTPUT == 1
		frames++;
#endif
	}

#if VERBOSE_OUTPUT == 1
	Serial.print("Ran GIF at "); Serial.print(frames / ((runStop - runStart) / 1000000.0f)); Serial.println(" fps");
#endif
	clearBuffer();
}

static void showGIF(uint8_t *data, int size, GIFDisplayOptions options)
{
	AnimatedGIF gif;
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(data, size, gifDrawBufferedLine))
	{
#if VERBOSE_OUTPUT == 1
	Serial.print("Opened streamed GIF with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		displayGIF(&gif, options);
		gif.close();
	}
	else
	{
		Serial.println("Could not open GIF from RAM");
	}
}

static void showGIF(const char *path, GIFDisplayOptions options)
{
	AnimatedGIF gif;
	gif.begin(BIG_ENDIAN_PIXELS);

	if (gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDrawBufferedLine))
	{
#if VERBOSE_OUTPUT == 1
	Serial.print("Opened GIF "); Serial.print(path); Serial.print(" with resolution "); Serial.print(gif.getCanvasWidth()); Serial.print(" x "); Serial.println(gif.getCanvasHeight());
#endif
		displayGIF(&gif, options);
		gif.close();
	}
	else
	{
		Serial.print("Could not open GIF "); Serial.println(path);
	}
}

static void showGIF(String path, GIFDisplayOptions options)
{
	showGIF(path.c_str(), options);
}

/*******************************************************************************
 * PNG
 *******************************************************************************/

#define TFT_TRANSPARENT_8BPP ((uint8_t)((TFT_TRANSPARENT & 0xE000)>>8 | (TFT_TRANSPARENT & 0x0700)>>6 | (TFT_TRANSPARENT & 0x0018)>>3))

static void pngDrawLine(PNGDRAW *pDraw)
{
	uint16_t lineBuffer[TFT_DISPLAY_MAX];
	PNG *png = ((PNG *)pDraw->pUser);
	png->getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, tft.color16to24(config.backgroundColor));
	displayBuffer.pushImage(xoffset, pDraw->y + yoffset, pDraw->iWidth, 1, lineBuffer, TFT_TRANSPARENT_8BPP);
}

static inline void displayPNG(PNG &png)
{
	displayState = DISPLAY_STATIC_IMAGE;

	int width = png.getWidth();
	int height = png.getHeight();
	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();
	xoffset = (displayWidth - width) / 2;
	yoffset = (displayHeight - height) / 2;

	clearBuffer();
	uint16_t *pixelBuf = (uint16_t *)displayBuffer.createSprite(displayWidth, displayHeight);
	displayBuffer.fillSprite(config.backgroundColor);

	png.decode(NULL, 0); // Fill displayBuffer sprite with image data

	if ((width < displayWidth || height < displayHeight) && !png.hasAlpha())
	{
		// When PNG is smaller than display and not transparent use the top-left pixel of the image for background
		uint32_t topLeftColor = displayBuffer.readPixel(xoffset, yoffset);
#if VERBOSE_OUTPUT == 1
		Serial.print("Filling screen with "); Serial.println(topLeftColor, HEX);
#endif
		displayBuffer.fillRect(0, 0, displayWidth, yoffset, topLeftColor); // Top space
		displayBuffer.fillRect(0, (displayHeight - height) / 2, (displayWidth - width) / 2, displayHeight - ((displayHeight - height) / 2), topLeftColor); // Left space
		displayBuffer.fillRect(xoffset + width, (displayHeight - height) / 2, displayWidth - (displayWidth - width / 2), displayHeight - ((displayHeight - height) / 2), topLeftColor); // Right space
		displayBuffer.fillRect(0, height + yoffset, displayWidth, displayHeight, topLeftColor); // Bottom space
	}

#if USE_DMA == 1
	tft.dmaWait();
	tft.pushPixelsDMA(pixelBuf, displayBufferCount);
#else
	displayBuffer.pushSprite(0, 0);
#endif
	clearBuffer();
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
		currentImage = String(path);
		currentImage.trim();
		currentImage.toLowerCase();

		bool found = false;
		for (int i = 0; i < imageExtensionCount; i++)
		{
			if (currentImage.endsWith(imageExtensions[i]))
			{
				currentGifOption.loop  = currentImage.indexOf(".loop") > -1;
				currentGifOption.uncap = currentImage.indexOf(".fast") > -1;
				found = true;
				break;
			}
		}

		if (found)
		{
			if (currentImage.endsWith(".gif"))
				showGIF(path, currentGifOption);
			else if (currentImage.endsWith(".png"))
				showPNG(path);
		}
		else
		{
			currentImage = "";
			showText(path);
		}
	}
	else showText(path);
}

void showImage(String path)
{
	showImage(path.c_str());
}

void showMister(void)
{
	showGIF((uint8_t *)mister_kun_blink, sizeof(mister_kun_blink), GIF_LOOP);
	displayState = DISPLAY_MISTER; // Explicitly set display state since the displayGIF() method set it
}

void showStartup(void)
{
	if (config.startupCommand != "")
	{
		CommandData command = CommandData::parseCommand(config.startupCommand);
		addToQueue(command);
	}
	else
	{
		int64_t end = millis() + config.startupDelay;
		while (millis() - end < 0)
		{
			showSystemInfo(millis());
			yield();
		}

		if (config.startupImage == "")
			showMister();
		else
			showImage(config.startupImage);
	}
}

/*******************************************************************************
 * Slideshow functions
 *******************************************************************************/

static void runSlideshow(uint32_t time)
{
	static uint32_t nextChange = 0;

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
		nextChange = time + config.slideshowDelay;
	}
}

/*******************************************************************************
 * Lifecycle functions
 *******************************************************************************/

void loopDisplay(uint32_t time)
{
	switch (displayState)
	{
		case DISPLAY_ANIMATED_GIF_LOOPING: showGIF(currentImage, currentGifOption); break;
		case DISPLAY_SLIDESHOW:            runSlideshow(time);                      break;
		case DISPLAY_SYSTEM_INFORMATION:   showSystemInfo(time);                    break;
		case DISPLAY_MISTER:               showMister();                            break;
		default:                                                                    break;
	}
}

#endif
