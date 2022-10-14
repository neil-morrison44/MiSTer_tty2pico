# tty2pico for Arduino

This is an Arduino platform version of tty2pico.

## Development

The project is configured to use PlatformIO for development targeting the [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core.

### Dependencies

The core and application dependencies will be automatically installed by PlatformIO when building the project. The external dependencies are:

* Adafruit SPIFlash
* Adafruit SdFat library
* AnimatedGIF
* JPEGDEC
* PNGdec
* TFT_eSPI

### PlatformIO Configuration

A PlatformIO build environment is defined for each supported board. The main `platformio.ini` file defines the shared build parameters for each enviroment and imports the environment config for each board from the `tty2pico/configs/env/` folder. The `tty2pico/configs/displays/` folder contains configurations for each supported display, and are also imported fromo the `platformio.ini` file. The shared configurations are then composed into build environments in each board's `env.ini` file.

### Board Configuration

Select a board configuration from the `Raspberyy Pi Pico/RP2040` boards package. If your board doesn't exist, you can likely use the `Generic RP2040` profile and select the appropriate flash size.

### Open Issues

* The JPEGDEC library forces an `FS.h` include which conflicts with the custom FS implementation in the Adafruit SdFat library. [This commit](https://github.com/FeralAI/JPEGDEC/commit/6c2143afc6aa7e6b10d7d80923d0bd81b94993e0) allows overriding this forced include.

## TODO

### Optimizations

#### File System Performance

The Adafruit SdFat library fork has a few drawbacks currently:

* It's based on the v1 branch of the mainline SdFat library
* It's about 20% slower for data reads than mainline SdFat v2

This is currently used for flash and SD card filesystem access. Consider a microSD only build until the performance issues can be addressed.

### Definitely TODO

* [x] Migrate project to PlatformIO
* [ ] Refine documentation and provide examples and pin definitions
* [ ] Implement support for whatever commands make sense from:
  * [tty2oled](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2)
    * [x] CMDBYE - Show Sorgelig's Cat Icon
    * [x] CMDCLS - Clear and Update the Display
    * [x] CMDCOR - Command to announce Corechange (no transitions)
    * [x] CMDDOFF - Switch Display off
    * [x] CMDDON - Power Display on
    * [ ] CMDGEO - Show Geometric Figures
    * [x] CMDROT - Rotate screen relative to starting position (0=none, 1=180°, 2=90°, 3=270°)
    * [x] CMDSAVER - Disable or Enable the ScreenSaver (currently only toggle)
    * [ ] CMDSETTIME - Set MCU clock, ESP32 only
    * [x] CMDSHTEMP - Alias to `CMDSHSYSHW` since that screen displays the CPU temp
    * [x] CMDSHSYSHW - Show tty2pico system information
    * [x] CMDSNAM - Show actual loaded Corename
    * [x] CMDSORG - Show Startup screen, also an alias to `CMDSHSYSHW`
    * [x] CMDSWSAVER - Switch Screensaver on or off
    * [x] CMDTEST - Show a full screen test picture
    * [x] CMDTXT - Send Text to the Display
  * [tty2tft](https://github.com/ojaksch/MiSTer_tty2tft/tree/main/doc#commands)
    * [ ] CMDDINVON - Invert screen on
    * [ ] CMDDINVOFF - Invert screen off
    * [ ] CMDVIDEOPLAY,PARAM - Play videos or not (yes/no/may)
* [x] Add GIF support
* [x] Add JPEG support
* [x] Add default assets
* [x] Support transparency (shows black for PNG)
  * [x] Allow setting up a background image/color
* [x] Add configuration for SPI Micro SD breakouts
* [x] More LCD/OLED display configurations
  * [x] SSD1351 128x128 OLED
  * [x] ST7735 128x160
  * [x] ST7789V3 172x320

### Maybe TODO

* Technical
  * [x] Multicore support (one for logic, the other for draw calls)
  * [ ] Move the "Software configuration" section of `config.h` into a text/json/xml/whatever file to be read from the filesystem on startup
* 3D Models
  * [ ] Create variant of existing GC9A01 holder for the RoundyPi
  * [ ] Modify the [MiSTer Multisystem dust cover](https://www.printables.com/model/159379-mister-multisystem-v5-2022-classic-gaming-console-/files) to support GC9A01/RoundyPi and possibly other display modules