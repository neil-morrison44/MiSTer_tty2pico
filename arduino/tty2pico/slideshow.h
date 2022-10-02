#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include "config.h"

#if defined(SLIDESHOW_ON_START) && SLIDESHOW_ON_START == 1
static bool slideshowActive = true;
#else
static bool slideshowActive = false;
#endif

bool isSlideshowActive(void)
{
  return slideshowActive;
}

void setSlideshowActive(bool active)
{
  slideshowActive = active;
}

void loopSlideshow(long time)
{
  if (slideshowActive)
    runSlideshowFrame(time);
}

#endif
