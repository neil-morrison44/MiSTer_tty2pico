# tty2pico for Arduino

A full colour version of [tty2oled](https://github.com/venice1200/MiSTer_tty2oled) display addon for the [MiSTer FPGA](https://github.com/MiSTer-devel) and features:

* Targets compatiblity with the [tty2oled Command List](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2)
* Supports PNG, static GIF, and animated GIF files at up to 50fps!
* Can display files from built-in flash or microSD card if available
* Support for SPI displays up to 320x240 resolution (may require manual build)
* Appears as USB Mass Storage device so you can easily load new files

## Hardware

Each RP2040 board and display combination requires its own build since a lot of the pin and bus configuration must be passed in at compile time. The mainline release targets the following hardware:

* Raspberry Pi Pico or equivalent generic board
* GC9A01 240x240 1.28inch IPS LCD Module
* Optional SPI microSD reader

The [RoundyPi](https://github.com/sbcshop/RoundyPi) module combines all three pieces of hardware on a single board, and is the recommended hardware to get started.

### MicroSD Cards

tty2pico supports loading files from a SPI-based microSD card reader. Cards up to 128GB have been tested to work, though larger likely will as well. The recommended and supported file system format for SD is exFAT. This will allow near instant loading of the tty2pico device as USB Mass Storage, which then allows the rest of the tty2pico application to run smoothly.

Using FAT32 from microSD does work, however it's not a supported format. The cluster size must be very large when formatting the card to ensure as little delay as possible when processing the 10's of thousands of SD reads it takes to mount a FAT volume, which our powerful but still limited MCU will struggle with. An 8GB card was test by formatting as FAT32 with 32k cluster size and it worked "OK", as in took several seconds to show as a drive on the computer but overall worked fine. While this may work, there honestly is no reason not to use exFAT for this application.

### Boards

Manual build configurations are available for the following RP2040 boards:

| Board | Flash Size | SD Reader? | Display? | Remarks |
| ----- | ---------- | ---------- | -------- | ------- |
| Raspberry Pi Pico | 2 MB | No | No | The original |
| Pico clones | 16 MB | No | No | e.g. Pimoroni Pico LiPo |
| [RoundyPi](https://github.com/sbcshop/RoundyPi) | 2 MB | Yes | Round 1.28" 240x240 GC9A01 | Just add an (optional) SD card! |
| Sparkfun Pro Micro RP2040 | 16 MB | No | No | Easily available and relatively cheap |
| Sparkfun Thing Plus RP2040 | 16 MB | Yes | No | A bit expensive but allows a lot of flexibility |

Each build is also preconfigured to use an external SPI-based SD reader if one isn't built-in. See the relevant `env/[BoardName].ini` file for pin mapping via the `SDCARD_` defines.

### Displays

In theory tty2pico can support any SPI display controller the TFT_eSPI library supports, though each display requires some custom setup via `build_flags` and its own build in the PlatformIO environment. See the [Development](#development) section for more information.

The development focus is on the round GC9A01 based display, though manual build configurations are available for the following displays:

| Resolution | Tech | Module | Driver |
| - | - | - | - |
| 240x240 Round | IPS | [1.28inch LCD Module](https://www.waveshare.com/wiki/1.28inch_LCD_Module) | GC9A01 |
| 320x172 | IPS | [1.47inch LCD Module](https://www.waveshare.com/wiki/1.47inch_LCD_Module) | ST7789V |
| 320x240 | TFT | [2.4inch LCD Module](https://www.waveshare.com/wiki/2.4inch_LCD_Module) | ILI9341 |
| 160x128 | TFT | [1.8inch LCD Module](https://www.waveshare.com/wiki/1.8inch_LCD_Module) | ST7735 |
| 128x128 | OLED | [1.5inch RGB OLED Module](https://www.waveshare.com/wiki/1.5inch_RGB_OLED_Module) | SSD1351 |

All testing has been done against Waveshare branded displays, aside from the RoundyPi. These are common display modules and you can find the same display modules from other brands.

## Configuration

tty2pico uses a config file in [TOML](https://toml.io/en/) format named `tty2pico.toml` at the root of your storage device for some options that can be adjusted at runtime. A sample `tty2pico.toml` file with all available options:

```toml
title = "tty2pico RoundyPi Configuration"

[tty2pico]
backgroundColor = 0
tftWidth = 240
tftHeight = 240
tftRotation = 2
overclockMode = 1
overclockSD = true
waitForSerial = false
imagePath = "/logos/"
startupCommand = ""
startupDelay = 5000
startupImage = ""
slideshowDelay = 2000
ttyBaudRate = 115200
```

And a description of each available option (struckthrough items are not yet implemented):

| Option | Valid Values | Default Value | Description |
| ------ | --------- | ------------- | ----------- |
| backgroundColor | 16-bit RGB565 color value in integer form | 0 (Black) | The default background color when using transparent images. You will need to find an RGB565 color value usually in hex format like [the TFT_eSPI color definitions](https://github.com/Bodmer/TFT_eSPI/blob/13e62a88d07ed6e29d15fe76b132a927ec29e307/TFT_eSPI.h#L282), then convert the hex value to an integer value using an online tool or the `tools/hex-to-int.py` Python script like `python hex-to-int.py FFFF` |
| overclockMode | 0 = Stock speed<br>1 = Basic Overclock<br>255 = [Ludicrous Speed](https://youtu.be/oApAdwuqtn8) (max tested overclock for the platform) | 0 | Set to `1` to double the clock speed of the RP2040 from 125MHz to 250MHz. This will provide almost a 2x performance increase for display refreshes and will allow well optimized GIFs to display at 50fps. Without an overclock 30fps is likely max, and there's no guarantee there.<br><br>For those that want to squeeze out every last drop of performance, the Ludicrious Speed setting will overclock the RP2040 to 266MHz! This works on a lot of boards, but sadly not the RoundyPi 😢<br><br>Just about every RP2040 board should be able to handle the basic overclock, and the RP2040 should not need any additional cooling, though keep it in mind if mounting your device inside an enclosure. |
| overclockSD | true/false | false | Some SD readers will not work with an overclocked SPI rate. Setting this option to false will throttle the SD SPI rate for better compatiblity.
| slideshowDelay | 0+ | 2000 | The delay in milliseconds between switching images during the slideshow/screensaver. |
| startupCommand | string | "" | The [tty2pico command](#command-list) to run at startup. |
| startupDelay | 0+ | 5000 | The delay in milliseconds to show the startup screen |
| startupImage | string | "" | The image to display after the `startupCommand` runs. |
| tftRotation | 0 = none<br>1 = 90°<br>2 = 180°<br>3 = 270° | Display specific | Override the default startup rotation of the display. NOT the same values as `CMDROT`. |
| tftHeight | 0-320 | Display specific | Override the native height of the display in pixels. If your screen is natively portrait (like the ST7789V) this value should be larger than `tftWidth`. |
| tftWidth | 0-320 | Display specific | Override the native width of the display in pixels. If your screen is natively portrait (like the ST7789V) this value should be smaller than `tftHeight`. |
| uncapFramerate | true/false | false | Allow animated GIFs to play without a framerate limit. |

## Command List

tty2pico aims to be as compatible as needed/possible with the [tty2oled v2 Command List](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2), along with adding some custom commands.

### tty2oled Commands

These commands are adapted from `tty2oled` and should be (mostly) compatible:

| Command | Function | Example |
| ------- | -------- | ------- |
| CMDBYE | Show Sorgelig's Cat Icon | `CMDBYE` |
| CMDCLS | Clear and Update the Display | `CMDCLS` |
| CMDCOR | Command to announce core change, will try to display in the following order:<br>`[corename].loop.gif`<br>`[corename].gif`<br>`[corename].png` | `CMDCOR,[corename]`<br>`[corename]`<br>e.g.<br>`SNES`<br>`CMDCOR,SNES`<br>`CMDCOR,19XX` |
| CMDDOFF | Switch Display off | `CMDDOFF` |
| CMDDON | Power Display on | `CMDDON` |
| CMDENOTA | Reboots tty2pico device into bootloader mode to receive a firmware update | `CMDENOTA` |
| CMDROT | Rotate screen relative to starting position (0=none, 1=180°, 2=90°, 3=270°) | `CMDROT,0` for no rotation<br>`CMDROT,1` to flip screen |
| CMDSAVER | Disable or Enable the ScreenSaver (currently only toggle) | `CMDSAVER` |
| CMDSETTIME | Set the time on the MCU RTC using a unix timestamp<br><br>Shell example: <br>`timeoffset=$(date +%:::z)`<br>`localtime=$(date '-d now '${timeoffset}' hour' +%s)`<br>`echo "CMDSETTIME,${localtime}" > /dev/ttyACM0` | `CMDSETTIME,[timestamp]`<br>`CMDSETTIME,1665971593` |
| CMDSHTEMP | Alias to `CMDSHSYSHW` since that screen displays the CPU temp | `CMDSHTEMP` |
| CMDSHSYSHW | Show tty2pico system information | `CMDSHSYSHW` |
| CMDSNAM | Show actual loaded Corename | `CMDSNAM` |
| CMDSORG | Show Startup screen, also an alias to `CMDSHSYSHW` | `CMDSORG` |
| CMDSWSAVER | Switch screensaver on or off | `CMDSWSAVER,0` to disable<br>`CMDSWSAVER,1` to enable |
| CMDTEST | Show system info and some test graphics | `CMDTEST` |
| CMDTXT | Send text to the display | `CMDTXT,Can you see this?`<br>Not currently compatible with `tty2oled` command |

### tty2pico Commands

These commands are specific to `tty2pico`:

| Command | Function | Example |
| ------- | -------- | ------- |
| CMDGETSYS | Retrieve a pipe-separated system identifier string composed from the build flags with the `TTY2PICO_` prefix that can be used to remotely manage tty2pico options and updates, example output:<br><br>`version=1.0.0\|board=Raspberry Pi Pico\|display=GC9A01` | `CMDGETSYS` |
| CMDGETTIME | Get the current real-time clock value in the specified format | `CMDGETTIME`<br>`CMDGETTIME,[format]`<br><br>Formats are:<br>0 = Unix timestamp (default if missing)<br>1 = Human readable |
| CMDSHOW | Show an image from the active storage device | `CMDSHOW,/logos/pattern.loop.gif` |

## Development

The project is configured to use PlatformIO for development targeting the [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core.

### PlatformIO Configuration

A PlatformIO build environment is defined for each supported board. The main `platformio.ini` file defines the shared build parameters for each enviroment and imports the environment config for each board from the `env/` folder. The `displays/` folder contains configurations for each supported display, and are also imported through the `platformio.ini` file. The shared configurations are then composed into build environments that are defined in each board's `env/[BoardName].ini` file.

If you would like to add a build for a board/display that isn't supported, copy one of the existing display or env files, rename, then update accordingly.

### Dependencies

All platform, framework and external library dependencies required to build will be automatically downloaded by PlatformIO when executing a build. The external library dependencies are:

* adafruit/Adafruit SPIFlash@4.0.0
* adafruit/Adafruit TinyUSB Library@1.14.4
* adafruit/SdFat - Adafruit Fork@2.2.1
* bitbank2/AnimatedGIF@1.4.7
* bitbank2/PNGdec@1.0.1
* bodmer/TFT_eSPI@2.4.78
* gyverlibs/UnixTime@1.1

## Roadmap

### TODO

* [x] ~~Migrate project to PlatformIO~~
* [x] ~~Refine documentation and provide examples and pin definitions~~
* [ ] Implement support for whatever commands make sense from:
  * [tty2oled](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2)
    * [x] ~~CMDBYE - Show Sorgelig's Cat Icon~~
    * [x] ~~CMDCLS - Clear and Update the Display~~
    * [x] ~~CMDCOR - Command to announce Corechange (no transitions)~~
    * [x] ~~CMDDOFF - Switch Display off~~
    * [x] ~~CMDDON - Power Display on~~
    * [ ] CMDGEO - Show Geometric Figures (maybe?)
    * [x] ~~CMDROT - Rotate screen relative to starting position (0=none, 1=180°, 2=90°, 3=270°)~~
    * [x] ~~CMDSAVER - Disable or Enable the ScreenSaver (currently only toggle)~~
    * [x] ~CMDSETTIME - Set MCU clock~
    * [x] ~~CMDSHTEMP - Alias to `CMDSHSYSHW` since that screen displays the CPU temp~~
    * [x] ~~CMDSHSYSHW - Show tty2pico system information~~
    * [x] ~~CMDSNAM - Show actual loaded Corename~~
    * [x] ~~CMDSORG - Show Startup screen, also an alias to `CMDSHSYSHW`~~
    * [x] ~~CMDSWSAVER - Switch Screensaver on or off~~
    * [x] ~~CMDTEST - Show a full screen test picture~~
    * [x] ~~CMDTXT - Send Text to the Display~~
  * [tty2tft](https://github.com/ojaksch/MiSTer_tty2tft/tree/main/doc#commands)
    * [ ] CMDDINVON - Invert screen on
    * [ ] CMDDINVOFF - Invert screen off
    * [ ] CMDVIDEOPLAY,PARAM - Play videos or not (yes/no/may)
* [x] ~~Add GIF support~~
* [x] ~~Add JPEG support~~
* [x] ~~Add default assets~~
* [x] ~~Support transparency (shows black for PNG)~~
  * [x] ~~Allow setting up a background image/color~~
* [x] ~~Add configuration for SPI Micro SD breakouts~~
* [x] ~~More LCD/OLED display configurations~~
  * [x] ~~SSD1351 128x128 OLED~~
  * [x] ~~ST7735 128x160~~
  * [x] ~~ST7789V3 172x320~~
* [ ] Multicore support (one for logic, the other for draw calls)
* [ ] Add support for other fast chips like ESP32 and ESP32-S3
* [x] ~Move the "Software configuration" section of `config.h` into a text/json/xml/whatever file to be read from the filesystem on startup~
* [ ] Create variant of existing GC9A01 holder for the RoundyPi
* [ ] Modify the [MiSTer Multisystem dust cover](https://www.printables.com/model/159379-mister-multisystem-v5-2022-classic-gaming-console-/files) to support GC9A01/RoundyPi and possibly other display modules

### Refactoring and Optimizations

**Remove Arduino String dependency**

Consider replacing instances of Arudino `String` with `std::string` or `const char *`, at least in platform-specific areas of code that could be modularized.

**Refactor Display Logic**

Display logic is in a bunch of global methods and tracking variables. We can make this more flexible by creating a base `Scene` class (or whatever naming) and then extend for each type of display logic, like `PngScene`, `GifScene`, `InfoScreenScene`, etc. This base display class has a static reference to the `TFT_eSprite` used as the display buffer, then we keep whatever `Scene` subclass in memory while it's used. This should allow easier composing of complex display sequences and transitions between display states.

**Replace Instances of `Serial` with `TTY_SERIAL`**

Use the define instead of the imported global object when intending to output data to client.
