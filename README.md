# MiSTer_tty2pico

tty2pico is an addon project for the [MiSTer FPGA](https://github.com/MiSTer-devel) which displays full color text, graphics or animations based on the active MiSTer core. It was inspired by, and strives for command compatibility with, the original [tty2oled](https://github.com/venice1200/MiSTer_tty2oled) project.

<p align="middle">
<img src="./readme_images/IMG_1520.jpeg" alt="Image of RoundyPi case" height="300">
<img src="./readme_images/8AEE547F-BAA3-452A-836F-03CCF3AC730D_1_105_c.jpeg" alt="Image of the display" height="300">
</p>


## Features

* Displays PNG, static GIF, and animated GIF files all with transparency!
* Tuned for performance - animations can play up to 50fps or more!
* Can display files from built-in flash or microSD card if available
* Support for SPI displays up to 320x240 resolution (may require advanced setup)
* Appears as USB Mass Storage device so you can maintain your files
* Targets compatiblity with the [tty2oled Command List](https://github.com/venice1200/MiSTer_tty2oled/wiki/Command_v2)

## Requirements

tty2pico releases target the following hardware:

* Raspberry Pi Pico or other pin-compatbile RP2040 board
* 1.28inch Round 240x240 GC9A01 LCD Module
* Optional SPI microSD reader and card

The [RoundyPi](https://github.com/sbcshop/RoundyPi) module combines all three pieces of hardware on a single board, and is the **recommended** hardware to use for your installation.

Several other board and display configurations are available for advanced setups, which are detailed in the [Hardware](https://github.com/neil-morrison44/MiSTer_tty2pico/wiki/Hardware) section of the Wiki.

## Quick Start

The quick start instructions assume the use of a RoundyPi, though most steps will be the same or very similar if using a Pico or another custom setup.

### Install Firmware

To install:

1. Download the `.uf2` for your setup from the (latest release)[https://github.com/neil-morrison44/MiSTer_tty2pico/releases/latest].
2. Hold the `BOOT` (or `BOOTSEL` on a Pico) button while plugging in your device. A new drive will appear on your computer with the name `RPI-RP2`.
3. Copy the `.uf2` file to the `RPI-RP2` drive. This will upload the new firmware to the device.

That's it! tty2pico should display a startup screen with some system information. If running from flash, tty2pico will try to mount an existing flash partition first. This will preserve your data between firmware updates. If no FAT partition is present on the flash parition it will be automatically created and labeled `TTY2PICO` when mounted as a drive on a PC. If running from SD card make sure it's using exFAT format and you're set.

If you'd like to build a custom setup take a look at the [Hardware](https://github.com/neil-morrison44/MiSTer_tty2pico/wiki/Hardware) page for a list of supported hardware and the [Development](https://github.com/neil-morrison44/MiSTer_tty2pico/wiki/Development) page for project setup information.

### Load Images

Images can either be loaded over USB from a computer, including the MiSTer, or directly on to SD card.

tty2pico supports PNG, transparent PNG, GIF and animated GIF files. When looking for an image to load, tty2pico will search the `/logos/` folder of your storage device (flash or microSD). Loading images is simple since your tty2pico device will show up as a flash drive on your computer. Just copy the images into the `/logos/` folder and you're set!

The logos should be named the same as the core, for example `snes.png` for SNES. This is not case-sensitive. If you don't know what the core name [check this list](https://mister-devel.github.io/MkDocs_MiSTer/developer/corenames/).

If you have a `[core name].gif` file it will play once, then stop (to support things that animate in, like the GAMEBOY logo) - if you want to loop the gif name it `[core name].loop.gif`, e.g. `snes.loop.gif`.

Make sure your image files are the same resolution or smaller than the display. See the [Image Files](https://github.com/neil-morrison44/MiSTer_tty2pico/wiki/Image-Files) section of the wiki for image specifications and optimization tips.

### (Optional) Edit tty2pico Configuration

On initial boot, tty2pico will generate a `tty2pico.toml` config file at the root of your storage device if one doesn't exist. Here you can tweak some default system options and tune a couple performance aspects of your setup. Refer to the [Configuration](https://github.com/neil-morrison44/MiSTer_tty2pico/wiki/Configuration) wiki page for a complete list of options.

By default tty2pico will overclock the RP2040 to ensure images load quickly & animate smoothly, this can be tweaked in the config - but it is recommended to run with the overclock if possible.

### MiSTer Setup

First follow installation instructions of <https://github.com/venice1200/MiSTer_tty2oled> to get the script running on your MiSTer.

tty2pico isn't fully compatible with tty2oled, so some features like `USBMODE` and script updates need to be disabled. Update these values in the .ini file:

* TTYDEV="/dev/ttyACM0" _(or whatever `tty` device the board appears as)_
* USBMODE="no"
* SCRIPT_UPDATE="no"
* TTY2OLED_UPDATE="no"

The `tty2pico` file from the releases should be put into the MiSTer's scripts folder, this auto-updating script will keep the board's firmware up to date and will, over time, allow for features to be added.

Finally plug in your tty2pico device and reboot the MiSTer to complete the setup.

## 3D printable cases

TBC (link to stl files once they're in and link to wiki page on printing)

### For the standalone RoundyPi

### For the RoundyPi + MiSTer Multisystem (V5)

### For the Pico


## Micropython version

The original, MicroPython version is under the `/micropython` folder in this repo.
It's not recommended for use, unless anyone particuarly likes the (unintentional) early 90s aesthetic of the images loading line by line.

## Thanks

- Thanks to (everyone who worked on tty2oled)[https://github.com/venice1200/MiSTer_tty2oled/graphs/contributors], since this project's standing on that project's shoulders (and, for the time being, scripts)
- Massive thanks to @FeralAI for getting the arduino version running. I'd tried to wrap my head around the pico's flash file system in C++ before & completely failed, compromising on MicroPython where that was dealt with already.
