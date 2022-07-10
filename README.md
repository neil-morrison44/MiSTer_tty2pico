# MiSTer_tty2pico
A full colour version of https://github.com/venice1200/MiSTer_tty2oled

- Should run on any device that supports micropython, tested on the RP2040 Pico.
- Built for the gc9a01 display, but swapping that for any other display should be easy enough
- The more RAM the device has got the faster it'll switch images

## Installation

On the board:
- If it's a Pi Pico you'll need to upload a custom firmware.uf2 file to support USB Mass Storage Class, hopefully I can get this building & added as an asset on release
- Find / create logo files which fit the following criteria:
  - _must_ be PNGs
  - max resolution depends on RAM but I've found ~160x160px works for the RP2040 Pico, it'll center the image regardless
  - should be run through something like imageOptim to compress the PNG
  - Remove the alpha channel
  - Not interlaced
  - Greyscale PNGs are supported and can probably be native display resolution since they're less resouce intensive
- Name the logo files the names of the cores (e.g. `logos/SNES.png`) this doesn't appear to be case sensitive
- Add them to the drive that appears when the device is mounted along with `main.py` and the `.mpy` versions of the other files


On the MiSTer side:
- Follow installation instructions of https://github.com/venice1200/MiSTer_tty2oled
- Change the tty in its `.ini` files to be the one for the device
- The device won't know what to do with the new updators etc added to tty2oled so... don't use them.

## Questions

### Why MicroPython?
I started writing this in C / C++ a while ago but gave up when I realised I'd have to do a lot of the USB MSC code myself to get it to appear as a flash drive, but if that changes it'd be a lot faster & probably support bigger images. CircuitPython is a bit more opinionated about what you can do with displays, which is probably nice if you're rendering a UI, but for this I just wanted to blit as fast as python could.

## TODO
- I'd like to have more default logos & have the error states (missing image, image too large) show something visually
- I'll design a case for the Pi Pico & gc9a01 and add the `.stl`s here
