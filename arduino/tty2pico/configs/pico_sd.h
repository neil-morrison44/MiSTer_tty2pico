#ifndef PICO_SD_CONFIG_H
#define PICO_SD_CONFIG_H

/********************************************************************************************
 * Configuration for Pico with external SPI MicroSD breakout, such as:
 * > SparkFun Level Shifting microSD Breakout - https://www.sparkfun.com/products/13743
 * > Treedix Level Shifting microSD Breakout - https://www.amazon.com/gp/product/B0B2HS1J63/
 *
 * NOTE: The CD (card detect) pin on the board is not currently used.
 ********************************************************************************************/

#define STORAGE_TYPE_SD_CARD

#define SDCARD_SPI       SPI
#define SDCARD_MOSI_PIN    3
#define SDCARD_MISO_PIN    4
#define SDCARD_SCK_PIN     2
#define SDCARD_CS_PIN      5

#endif
