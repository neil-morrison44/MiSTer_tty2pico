#ifndef ST7735_128_160_H
#define ST7735_128_160_H

#define ST7735_DRIVER
#define ST7735_GREENTAB2
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

/* Select speed */
// #define SPI_FREQUENCY 80000000 // 80 MHz
// #define SPI_FREQUENCY 66500000 // 66.5 MHz - Max frequency for RP2040 @133MHz
// #define SPI_FREQUENCY 62500000 // 62.5 MHz - Max frequency for RP2040 @125MHz (default)
#define SPI_FREQUENCY 40000000 // 40 MHz
// #define SPI_FREQUENCY 27000000 // 27 MHz
// #define SPI_FREQUENCY 20000000 // 20 MHz
// #define SPI_FREQUENCY 16000000 // 16 MHz

#endif
