# tty2pico for Arduino

This is an Arduino platform version of tty2pico.

## Versions

There are two versions that can be built and loaded: Flash Filesystem (FS) or Micro SD.

### Flash FS

The Flash FS version has a minimum overhead of 1 MB for the sketch, so a standard Pico (with 2 MB flash) will only allow about 1 MB of files to be stored. There are other Pico-like boards out there with up to 16 MB of flash, and 15 MB is plenty of storage for a full set of console and arcade images if they're well optimized. Boards like the Sparkfun Pro Micro RP2040, Sparkfun Thing Plus RP2040, Pimoroni Pico LiPo and all verified to work and allow the max of 15 MB to be used.

A fringe benefit of the reserved space for the sketch code is that the flash filesystem stays intacts between flashing new versions of tty2pico.

### Micro SD

The Micro SD version supports SPI readers and cards using a FAT filesystem. Cards up to 8 GB have been verified to work, though larger sizes may work as well. The prototypical "holy grail" device for this version would be the [RoundyPi](https://www.amazon.com/RoundyPi-RoundyFi-Compact-Display-ESP-12E/dp/B0B297J6LB), which is an all-in-one round 240x240 GC9A01 display board with integrated RP2040 and Micro SD slot.

## Building

### Arduino IDE Setup

This project uses the [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core. Set it up [per the instructions](https://arduino-pico.readthedocs.io/en/latest/install.html#installing-via-arduino-boards-manager).

Once installed, select a board configuration from the `Raspberyy Pi Pico/RP2040` section. If your board doesn't exist, you can likely use the `Generic RP2040` profile and select the appropriate flash size.

If you are building the version to run from flash then change the `USB Stack` to `Adafruit TinyUSB` so the MSC code will work.

### Dependencies

These are the shared dependencies between all builds from the Arduino Library Manager:

* PNGdec
* TFT_eSPI

These dependencies need to be installed manually:

* [TFT_eSPI_Configs_RP2040](https://github.com/FeralAI/TFT_eSPI_Configs_RP2040)

#### Flash FS Version Dependencies

From Arduino Library Manager:

* Adafruit_SPIFlash
* Adafruit_TinyUSB_Library
* SdFat_-_Adafruit_Fork

#### Micro SD Version Dependencies

> WARNING: The `File` implementation in `SdFat_-_Adafruit_Fork` is incompatible with the version in the SD backend of the Arduino-Pico core. The `SdFat_-_Adafruit_Fork` folder will need to be moved if building the SD version until a better solution is found.

There are no additional dependencies outside of the Arduino-Pico core and the previously defined shared libraries.

### Configuration

All program configuration and most hardware configuration is handled in the `config.h`, and can be overridden from the `tty2pico.ino` file prior to importing `config.h`. There are prebuilt configuations to make setting up the display and optionally a Micro SD reader much simpler. Define one of these at the top of the `.ino` file:

* `CONFIG_PICO` - Flash FS version for Raspberry Pi Pico or clone board with up to 16MB (15MB usable) of flash storage
* `CONFIG_ROUNDYPI` - Micro SD version for Roundy Pi
* `CONFIG_THINGPLUS` - Micro SD version for Sparkfun Thing Plus RP2040

Each of these defines corresponds to a config header in the `configs` folder. If you want to add your own config just create a new header in the `configs` folder and add an options to the `#if..#elif..#else` import block in `config.h`.

#### Configuring TFT_eSPI

Configuration of `TFT_eSPI` is handled inside of its library folder. To make things easier, the `libraries/TFT_eSPI_Configs_RP2040` folder contains prebuilt configs for certain RP2040 board and display combinations. Copy the `TFT_eSPI_Configs_RP2040` folder into your Arduino libaries folder.

The `User_Setup_Select.h` file in the `TFT_eSPI` library folder needs to be modified to point to a custom configuration from the `TFT_eSPI_Configs_RP2040` folder that was just populated. Comment out any `#include` lines in the `USER_SETUP_LOADED` section at the top of the file, then add a new `#include` line to point to the configuration you want to use, something like:

```c
#ifndef USER_SETUP_LOADED //  Lets PlatformIO users define settings in
                          //  platformio.ini, see notes in "Tools" folder.

// Only ONE line below should be uncommented.  Add extra lines and files as needed.

// #include <User_Setup.h>           // Default setup is root library folder

#include <../TFT_eSPI_Configs_RP2040/GC9A01_RoundyPi.h>
```

## TODO

### Definitely TODO

* [ ] Refine documentation and provide examples and pin definitions
* [ ] Implement support for whatever commands make sense from [tty2oled](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2) and [tty2tft](https://github.com/ojaksch/MiSTer_tty2tft/tree/main/doc#commands)
* [ ] Add GIF support
* [ ] Add JPEG support
* [ ] Add default assets
* [ ] Support transparency (shows black for PNG)
  * [ ] Allow setting up a background image/color
* [ ] Add configuration for SPI Micro SD breakouts
* [ ] More LCD/OLED display configurations
  * [ ] ST7735 128x160
  * [ ] ST7789V3 172x320

### Maybe TODO
* Technical
  * [ ] Multicore support (one for logic, the other for draw calls)
  * [ ] Move the "Software configuration" section of `config.h` into a text/json/xml/whatever file to be read from the filesystem on startup
* 3D Models
  * [ ] Create variant of existing GC9A01 holder for the RoundyPi
  * [ ] Modify the [MiSTer Multisystem dust cover](https://www.printables.com/model/159379-mister-multisystem-v5-2022-classic-gaming-console-/files) to support GC9A01/RoundyPi and possibly other display modules