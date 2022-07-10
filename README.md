# MiSTer_tty2pico
A full colour version of https://github.com/venice1200/MiSTer_tty2oled

- Should run on any device that supports micropython, tested on the RP2040 Pico.
- Built for the gc9a01 display, but swapping that for any other display should be easy enough
- The more RAM the device has got the faster it'll switch images

## Installation
Hardware:
- Connect the display to match the Pins specified in https://github.com/neil-morrison44/MiSTer_tty2pico/blob/main/tty_and_logos.py#L17-L22
- Plug in to USB with BOOTSEL held

On the board:
- If it's a Pi Pico you'll need to upload a custom firmware.uf2 file to support USB Mass Storage Class, this can be found attached to the release (can see the workflow that adds it in this repo)
- Once it's installed the firmware it should appear as an empty flash drive (might need to unplug & replug), extract and copy over the files found in `files.zip` on the release
- Find / create logo files which fit the following criteria:
  - _must_ be PNGs
  - max resolution depends on RAM but I've found ~160x160px works for the RP2040 Pico, it'll center the image regardless
  - should be run through something like imageOptim to compress the PNG
  - Remove the alpha channel
  - Not interlaced
  - Greyscale PNGs are supported and can probably be native display resolution since they're less resouce intensive
- Name the logo files the names of the cores (e.g. `logos/SNES.png`) this doesn't appear to be case sensitive
- Add them to the drive that appears when the device is mounted along with `main.py` and the `.mpy` versions of the other files. (to add them once it's plugged in to the MiSTer you'll need to do
```
$ sudo mkdir /media/pico-usb
$ sudo mount /dev/sda /media/pico-usb
```
(assuming the drive is at `/dev/sda`)


On the MiSTer side:
- Follow installation instructions of https://github.com/venice1200/MiSTer_tty2oled
- Change the tty in its `.ini` files to be the one for the device. Also turn off unsupported features. e.g.
```
TTYDEV="/dev/ttyACM0"
USBMODE="no"
SCRIPT_UPDATE="no"
TTY2OLED_UPDATE="no"
```
- The device won't know what to do with the new updators etc added to tty2oled so... don't use them.

## For other boards
The `.mpy` files & `firmware.uf2` are intended for the Pi Pico, but the `.py` files themselves should work on any board that can run micropython (though it'll need to support MSC mode & the RAM will be more limited than when using `.mpy` files).
Custom firmware / `.mpy` files for other boards / architectures can be added here fairly easily.

## Questions

### How do I know what to name the logo files?
Once you've got the board connected and running you can ssh in and type
```
$ screen /dev/ttyACM0
```

Which'll show you what the MiSTer is sending, what filename was attempted, and a couple of error messages.
The filesystem is case insensitive, so `psx.png` should be found when it sends `PSX` etc

### Why MicroPython?
I started writing this in C / C++ a while ago but gave up when I realised I'd have to do a lot of the USB MSC code myself to get it to appear as a flash drive, but if that changes it'd be a lot faster & probably support bigger images. CircuitPython is a bit more opinionated about what you can do with displays, which is probably nice if you're rendering a UI, but for this I just wanted to blit as fast as python could.

### Why not include logos in this repo?
I'm not a fan of being sued, so unless someone can justify their fair use I'll just leave the default `mister` one in (which I'm not even sure of the copyright on but it's used elsewhere so...).

There might be a set of `.pngs` somewhere, _eventually_, but it won't be here.

### Can it support <screen X>?
Sure, just edit the `tty_and_logos.py` file to point to a different display lib & change some `240`s into whatever the new display width & height is, so long as it supports the `.blit_buffer` method it should work fine.

## TODO
- I'd like to have more default logos & have the error states (missing image, image too large) show something visually
- I'll design a case for the Pi Pico & gc9a01 and add the `.stl`s here
- Could do my own updater & watch script rather than piggybacking off of tty2oled
- Custom PCB? Probably a bit much.

## Libs
- Uses https://github.com/russhughes/gc9a01py with some tweaks to add `@micropython.native` to a couple of methods & remove the unused text rendering
- Uses https://github.com/Ratfink/micropython-png , which requires the itertools module
