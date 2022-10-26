#ifndef TTY2PICO_PLATFORM_H
#define TTY2PICO_PLATFORM_H

#include "definitions.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "platforms/esp32.h"
#else // ARDUINO_ARCH_RP2040
#include "platforms/rp2040.h"
#endif
#include "SpiDriver/SdSpiDriver.h"

inline void pauseBackground(void);
inline void resumeBackground(void);
float getCpuSpeedMHz(void);
float getCpuTemperature(void);
SdSpiConfig getSdSpiConfig(void);
const char *getTime(int format);
float getSpiRateDisplayMHz();
float getSpiRateSdMHz();
void setTime(uint32_t timestamp);
void resetForUpdate(void);
void setupPlatform(bool (*checkSDCallback)());
void setupQueue(void);
void addToQueue(CommandData &data);
bool removeFromQueue(CommandData &data);

#endif
