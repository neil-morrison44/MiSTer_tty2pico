#define GC9A01_DRIVER

#define RP2040_PIO_SPI

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

// #define TFT_SPI_PORT 0
#define TFT_MOSI 23
#define TFT_SCLK 22
#define TFT_CS    4
#define TFT_DC    5
#define TFT_RST   6
#define TFT_BL    7
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY  66500000 // Max for 133 MHz speed
// #define SPI_FREQUENCY  62500000 // Max for 125 MHz speed
// #define SPI_FREQUENCY  40000000 // Standard speed

// #define SPI_READ_FREQUENCY  5000000

// #define SPI_TOUCH_FREQUENCY  2500000

// #define SUPPORT_TRANSACTIONS
