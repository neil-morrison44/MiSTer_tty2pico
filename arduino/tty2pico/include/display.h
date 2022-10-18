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
	".loop.gif",
	".gif",
	".png",
};
const int imageExtensionCount = sizeof(imageExtensions) / sizeof(imageExtensions[0]);

int32_t xoffset = 0;
int32_t yoffset = 0;
String currentImage;
static DisplayState displayState = DISPLAY_STATIC_IMAGE;

/*******************************************************************************
 * Display setup
 *******************************************************************************/

TFT_eSPI tft = TFT_eSPI(config.tftWidth, config.tftHeight);
TFT_eSprite displayBuffer(&tft);

void setupDisplay()
{
#if defined(TFT_BL)
	pinMode(TFT_BL, OUTPUT);
	digitalWrite(TFT_BL, LOW); // Turn off backlight before starting up screen
#endif

	// Setup screen
	tft.init();
#if USE_DMA == 1
	tft.initDMA();
#endif
	tft.setRotation(config.tftRotation);
	tft.setTextFont(2);
	tft.setTextWrap(true);

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
	tft.fillScreen(config.backgroundColor);
}

/*******************************************************************************
 * Text and Graphics functions
 *******************************************************************************/

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
	int squareX = displayWidth / 2;
	const int minFrameCount = -(squareSize / 2);
	const int maxFrameCount = displayWidth + (squareSize / 2);
	int frame = -squareSize;
	bool reverse = false;

	displayBuffer.createSprite(displayWidth, config.getDisplayHeight());
	while (millis() < endTime)
	{
		displayBuffer.fillScreen(TFT_BLACK);
		displayBuffer.fillCircle(frame, displayWidth - frame, circleRadius, TFT_RED);
		displayBuffer.fillRoundRect(displayWidth - (squareSize / 2) - frame, frame - (squareSize / 2), squareSize, squareSize, 3, TFT_GREEN);
		displayBuffer.fillTriangle(frame, frame - triangleSize, frame - triangleSize, frame + triangleSize, frame + triangleSize, frame + triangleSize, TFT_BLUE);
		displayBuffer.fillEllipse(displayWidth - frame, displayWidth - frame, ellipseRadiusX, ellipseRadiusY, TFT_YELLOW);
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
	Serial.print("Demo random shapes at "); Serial.print(frames / (runtime / 1000000.0f)); Serial.println(" fps");
#endif
	displayBuffer.deleteSprite();
}

// Draws a transparent overlay on top of whatever is in the display buffer
void overlayText(String text, TFT_eSPI *parent)
{
	int font = config.getFontSmall();
	TFT_eSprite overlay(parent);
	int width = tft.textWidth(text, font) + (DISPLAY_TEXT_MARGIN * 2);
	int height = tft.fontHeight(font) + (DISPLAY_TEXT_MARGIN * 2);
	overlay.createSprite(width, height);
	overlay.fillSprite(TFT_TRANSPARENT);
	overlay.setTextColor(TFT_WHITE);
	overlay.setTextFont(font);
	overlay.drawString(text, DISPLAY_TEXT_MARGIN, DISPLAY_TEXT_MARGIN, font);
	overlay.pushSprite((config.getDisplayWidth() - width) / 2, (config.getDisplayHeight() - height) / 2);
	overlay.deleteSprite();
}

void overlayText(String text)
{
	overlayText(text, &tft);
}

#if SHOW_FPS == 1
static float fps;

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
	displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());
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

	displayBuffer.pushSprite(0, 0);
	displayBuffer.deleteSprite();

	displayState = DISPLAY_STATIC_TEXT;
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
	displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayHeight());
	displayBuffer.fillSprite(TFT_BLACK);
	displayBuffer.setTextColor(TFT_WHITE);

	int midpointX = config.getMidpointX();
	int midpointY = config.getMidpointY();
	int start = -(lineCount / 2);
	int yOffset = 0;

	for (int i = 0; i < lineCount; i++)
	{
		yOffset = ((i == 0) ? config.getFontLargeSize() : 0) + (-(i + start + 1) * config.getFontSmallSize()) + DISPLAY_TEXT_MARGIN;
		displayBuffer.drawCentreString(lines[i], midpointX, midpointY - yOffset, (i == 0) ? config.getFontLarge() : config.getFontSmall());
	}

	displayBuffer.pushSprite(0, 0);
	displayBuffer.deleteSprite();

	displayState = DISPLAY_STATIC_TEXT;
}

// Display a frame of the system information screen
void showSystemInfo(uint32_t frameTime)
{
	static uint32_t nextFrameTime;

	int32_t frameTimeDiff = frameTime - nextFrameTime;
	if (frameTimeDiff < 0)
		return;

	vector<String> lines;
	char lineBuffer[50];
	lines.push_back("tty2pico");
	lines.push_back("v" + String(TTY2PICO_VERSION_STRING));
	lines.push_back(TTY2PICO_BOARD);

	snprintf(lineBuffer, sizeof(lineBuffer), "RP2040 @ %.1fMHz %.1fC", (clock_get_hz(clk_sys) / 1000000.0f), analogReadTemp());
	lines.push_back(lineBuffer);

	memset(lineBuffer, 0, sizeof(lineBuffer));
	snprintf(lineBuffer, sizeof(lineBuffer), "%s @ %dx%d %.1fMHz", TTY2PICO_DISPLAY, config.getDisplayWidth(), config.getDisplayHeight(), getSpiRateDisplayMHz());
	lines.push_back(lineBuffer);

	lines.push_back(String(getTime(DTF_HUMAN)));

	if (getHasSD())
	{
		memset(lineBuffer, 0, sizeof(lineBuffer));
		snprintf(lineBuffer, sizeof(lineBuffer), "SD Filesystem @ %.1fMHz", getSpiRateSdMHz(), getMscReady() ? "Enabled" : "Disabled");
		lines.push_back(lineBuffer);
	}
	else lines.push_back("Flash Filesystem");

	if (getMscReady())
		lines.push_back("USB MSC Enabled");
	else
		lines.push_back("USB MSC Disabled");

	showHeaderedText(lines.data(), lines.size());

	displayState = DISPLAY_SYSTEM_INFORMATION;
	nextFrameTime = frameTime + 1000; // Only update display once every second
}

/*******************************************************************************
 * GIF
 *******************************************************************************/

#ifdef USE_GIF_BUFFERING
static void gifDrawLine(GIFDRAW *pDraw)
{
	uint16_t usTemp[1][config.getLineBufferSize()]; // Global display buffer

	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, y, iWidth, iCount;

	// Display bounds check and cropping
	int displayWidth = config.getDisplayWidth();
	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > displayWidth)
		iWidth = displayWidth - pDraw->iX;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line
	if (y >= config.getDisplayHeight() || pDraw->iX >= displayWidth || iWidth < 1)
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
			while (color != ucTransparent && s < pEnd && iCount < config.getLineBufferSize())
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
			if (iWidth <= config.getLineBufferSize())
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < config.getLineBufferSize(); iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

			displayBuffer.pushImage(pDraw->iX + iCount + xoffset, y + yoffset, iCount, 1, &usTemp[0][0]);
			iWidth -= iCount;
		}
	}
}

static inline void displayGIF(AnimatedGIF *gif, bool loop = false)
{
	int width = gif->getCanvasWidth();
	int height = gif->getCanvasHeight();
	xoffset = (config.getDisplayWidth() - width) / 2;
	yoffset = (config.getDisplayHeight() - height) / 2;
	displayState = loop ? DISPLAY_ANIMATED_GIF_LOOPING : DISPLAY_ANIMATED_GIF;

	displayBuffer.createSprite(config.getDisplayWidth(), config.getDisplayWidth());

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
	Serial.print("Ran bufferred GIF "); Serial.print(" at "); Serial.print(frames / (runtime / 1000000.0f)); Serial.println(" fps");
#endif

	displayBuffer.deleteSprite();
}
#else
// From the examples in the https://github.com/bitbank2/AnimatedGIF repo
// Draw a line of image directly on the LCD
static void gifDrawLine(GIFDRAW *pDraw)
{
	static int bufferSize;
	bufferSize = config.getLineBufferSize();

	#if USE_DMA == 1
	uint16_t usTemp[2][bufferSize];
	#else
	uint16_t usTemp[1][bufferSize];
	#endif
	bool dmaBuf = 0;

	uint8_t *s;
	uint16_t *d, *usPalette;
	int x, y, iWidth, iCount;

	// Display bounds check and cropping
	int displayWidth = config.getDisplayWidth();
	iWidth = pDraw->iWidth;
	if (iWidth + pDraw->iX > displayWidth)
		iWidth = displayWidth - pDraw->iX;
	usPalette = pDraw->pPalette;
	y = pDraw->iY + pDraw->y; // current line
	if (y >= config.getDisplayHeight() || pDraw->iX >= displayWidth || iWidth < 1)
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
			while (c != ucTransparent && s < pEnd && iCount < bufferSize)
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
#if USE_DMA == 1
				tft.dmaWait();
				tft.setAddrWindow(pDraw->iX + x + xoffset, y + yoffset, iCount, 1);
				tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
#else
				tft.setAddrWindow(pDraw->iX + x + xoffset, y + yoffset, iCount, 1);
				tft.pushPixels(usTemp, iCount);
#endif
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
		if (iWidth <= bufferSize)
			for (iCount = 0; iCount < iWidth; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];
		else
			for (iCount = 0; iCount < bufferSize; iCount++)
				usTemp[dmaBuf][iCount] = usPalette[*s++];

		tft.dmaWait();
		tft.setAddrWindow(pDraw->iX + xoffset, y + yoffset, iWidth, 1);
		tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
		dmaBuf = !dmaBuf;
#else
		tft.setAddrWindow(pDraw->iX + xoffset, y + yoffset, iWidth, 1);
		tft.pushPixels(usTemp, iCount);
#endif

		iWidth -= iCount;
		// Loop if pixel buffer smaller than width
		while (iWidth > 0)
		{
			// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
			if (iWidth <= bufferSize)
				for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
			else
				for (iCount = 0; iCount < bufferSize; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#if USE_DMA == 1
			tft.dmaWait();
			tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
			dmaBuf = !dmaBuf;
#else
			tft.pushPixels(usTemp, iCount);
#endif
			iWidth -= iCount;
		}
	}
}

static inline void displayGIF(AnimatedGIF *gif, bool loop = false)
{
	xoffset = (config.getDisplayWidth() - gif->getCanvasWidth()) / 2;
	yoffset = (config.getDisplayHeight() - gif->getCanvasHeight()) / 2;
	displayState = loop ? DISPLAY_ANIMATED_GIF_LOOPING : DISPLAY_ANIMATED_GIF;
	tft.startWrite();

#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
	int runStart = micros();
	int frames = 0;
#endif
#if SHOW_FPS == 1
	int frameStart = micros();
#endif

	while (gif->playFrame(true, NULL))
	{
#if SHOW_FPS == 1
		showFPS(micros() - frameStart);
		frames++;
#endif
#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
		frames++;
#endif
		delay(0); // delay(0) "yields" better performance than a yield()
#if SHOW_FPS == 1
		frameStart = micros();
#endif
	}

#if SHOW_FPS == 1 || VERBOSE_OUTPUT == 1
	if (gif->getLastError() == GIF_SUCCESS)
	{
	#if SHOW_FPS == 1
		showFPS(micros() - frameStart);
	#endif
	#if VERBOSE_OUTPUT == 1
		frames++;
	#endif
	}
#endif
#if VERBOSE_OUTPUT == 1
	Serial.print("Ran GIF at "); Serial.print(frames / ((micros() - runStart) / 1000000.0f)); Serial.println(" fps");
#endif

	if (displayState == DISPLAY_ANIMATED_GIF_LOOPING)
		gif->reset();
	else
		gif->close();

	tft.endWrite();
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
	uint16_t lineBuffer[config.getLineBufferSize()];
	PNG *png = ((PNG *)pDraw->pUser);
	bool hasAlpha = png->hasAlpha();
	png->getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, tft.color16to24(config.backgroundColor));
	displayBuffer.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

static inline void displayPNG(PNG &png)
{
	int width = png.getWidth();
	int height = png.getHeight();
	int displayWidth = config.getDisplayWidth();
	int displayHeight = config.getDisplayHeight();
	xoffset = (displayWidth - width) / 2;
	yoffset = (displayHeight - height) / 2;
	displayState = DISPLAY_STATIC_IMAGE;

	displayBuffer.createSprite(width, height);
	displayBuffer.fillSprite(TFT_TRANSPARENT);

	png.decode(NULL, 0); // Fill displayBuffer sprite with image data

	if (width < displayWidth || height < displayHeight || !png.hasAlpha())
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
	if (config.startupCommand != "")
	{
		CommandData command = CommandData::parseCommand(config.startupCommand);
		addToQueue(command);
	}
	else
	{
		showSystemInfo(millis());
		delay(config.startupDelay);

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
		case DISPLAY_ANIMATED_GIF_LOOPING: showGIF(currentImage, true);   break;
		case DISPLAY_SLIDESHOW:            runSlideshow(time);            break;
		case DISPLAY_SYSTEM_INFORMATION:   showSystemInfo(time);          break;
		case DISPLAY_MISTER:               showMister();                  break;
		default:                                                          break;
	}
}

#endif
