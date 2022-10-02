#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include "config.h"
#include "storage.h"
#include "display.h"

#if defined(SLIDESHOW_ON_START) && SLIDESHOW_ON_START == 1
static bool slideshowActive = true;
#else
static bool slideshowActive = false;
#endif

static void runSlideshowFrame(long time)
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
		showPng(nextFile);
		nextChange = time + SLIDESHOW_DELAY;
	}
}

void loopSlideshow(long time)
{
	if (slideshowActive)
		runSlideshowFrame(time);
}

bool isSlideshowActive(void)
{
	return slideshowActive;
}

void setSlideshowActive(bool active)
{
	slideshowActive = active;
}

#endif
