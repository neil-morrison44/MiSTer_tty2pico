"""
Copyright (c) 2020, 2021 Russ Hughes

This file incorporates work covered by the following copyright and
permission notice and is licensed under the same terms:

The MIT License (MIT)

Copyright (c) 2019 Ivan Belokobylskiy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


GC9A01 Display driver in MicroPython based on devbis st7789py_mpy module from
https://github.com/devbis/st7789py_mpy modified to drive 240x240 pixel GC9A01
displays.

The driver supports display rotation, mirroring, scrolling and drawing text
using 8 and 16 bit wide bitmap fonts with heights that are multiples of 8.
Included are 12 bitmap fonts derived from classic pc text mode fonts and a
couple of example programs.

This is a work in progress. Documentation can be found at
https://penfold.owt.com/gc9a01py/.

If you are looking for a faster driver with additional features, check out the
C version of the driver at https://github.com/russhughes/gc9a01_mpy

"""

# pylint: disable=invalid-name,import-error

import time
from micropython import const
import ustruct as struct

# commands
GC9A01_SWRESET = const(0x01)
GC9A01_SLPIN = const(0x10)
GC9A01_SLPOUT = const(0x11)
GC9A01_INVOFF = const(0x20)
GC9A01_INVON = const(0x21)
GC9A01_DISPOFF = const(0x28)
GC9A01_DISPON = const(0x29)
GC9A01_CASET = const(0x2A)
GC9A01_RASET = const(0x2B)
GC9A01_RAMWR = const(0x2C)
GC9A01_VSCRDEF = const(0x33)
GC9A01_COLMOD = const(0x3A)
GC9A01_MADCTL = const(0x36)
GC9A01_VSCSAD = const(0x37)

# Color definitions
BLACK = const(0x0000)
BLUE = const(0x001F)
RED = const(0xF800)
GREEN = const(0x07E0)
CYAN = const(0x07FF)
MAGENTA = const(0xF81F)
YELLOW = const(0xFFE0)
WHITE = const(0xFFFF)

_ENCODE_PIXEL = ">H"
_ENCODE_POS = ">HH"

_BUFFER_SIZE = const(256)

ROTATIONS = [
    0x48,   # 0 - PORTRAIT
    0x28,   # 1 - LANDSCAPE
    0x88,   # 2 - INVERTED_PORTRAIT
    0xe8,   # 3 - INVERTED_LANDSCAPE
    0x08,   # 4 - PORTRAIT_MIRRORED
    0x68,   # 5 - LANDSCAPE_MIRRORED
    0xc8,   # 6 - INVERTED_PORTRAIT_MIRRORED
    0xa8]   # 7 - INVERTED_LANDSCAPE_MIRRORED]


def color565(red: int, green: int = 0, blue: int = 0) -> int:
    """
    Convert red, green and blue values (0-255) into a 16-bit 565 encoded color.
    """
    try:
        red, green, blue = red  # see if the first var is a tuple/list
    except TypeError:
        pass
    return (red & 0xf8) << 8 | (green & 0xfc) << 3 | blue >> 3


def _encode_pos(x, y):
    """Encode a postion into bytes."""
    return struct.pack(_ENCODE_POS, x, y)


def _encode_pixel(color):
    """Encode a pixel color into bytes."""
    return struct.pack(_ENCODE_PIXEL, color)


class GC9A01():
    """
    GC9A01 driver class

    Args:
        spi (spi): spi object (Required)
        dc (pin): dc pin (Required)
        cs (pin): cs pin {optional}
        reset (pin): reset pin
        backlight(pin): backlight pin
        rotation (int): display rotation
    """

    def __init__(
            self,
            spi=None,
            dc=None,
            cs=None,
            reset=None,
            backlight=None,
            rotation=0):
        """
        Initialize display.
        """
        if spi is None:
            raise ValueError("SPI object is required.")

        if dc is None:
            raise ValueError("dc pin is required.")

        self.width = 240
        self.height = 240
        self.spi = spi
        self.reset = reset
        self.dc = dc
        self.cs = cs
        self.backlight = backlight
        self._rotation = rotation % 8

        self.hard_reset()
        time.sleep_ms(100)

        self._write(0xEF)
        self._write(0xEB, b'\x14')
        self._write(0xFE)
        self._write(0xEF)
        self._write(0xEB, b'\x14')
        self._write(0x84, b'\x40')
        self._write(0x85, b'\xFF')
        self._write(0x86, b'\xFF')
        self._write(0x87, b'\xFF')
        self._write(0x88, b'\x0A')
        self._write(0x89, b'\x21')
        self._write(0x8A, b'\x00')
        self._write(0x8B, b'\x80')
        self._write(0x8C, b'\x01')
        self._write(0x8D, b'\x01')
        self._write(0x8E, b'\xFF')
        self._write(0x8F, b'\xFF')
        self._write(0xB6, b'\x00\x00')
        self._write(0x3A, b'\x55')
        self._write(0x90, b'\x08\x08\x08\x08')
        self._write(0xBD, b'\x06')
        self._write(0xBC, b'\x00')
        self._write(0xFF, b'\x60\x01\x04')
        self._write(0xC3, b'\x13')
        self._write(0xC4, b'\x13')
        self._write(0xC9, b'\x22')
        self._write(0xBE, b'\x11')
        self._write(0xE1, b'\x10\x0E')
        self._write(0xDF, b'\x21\x0c\x02')
        self._write(0xF0, b'\x45\x09\x08\x08\x26\x2A')
        self._write(0xF1, b'\x43\x70\x72\x36\x37\x6F')
        self._write(0xF2, b'\x45\x09\x08\x08\x26\x2A')
        self._write(0xF3, b'\x43\x70\x72\x36\x37\x6F')
        self._write(0xED, b'\x1B\x0B')
        self._write(0xAE, b'\x77')
        self._write(0xCD, b'\x63')
        self._write(0x70, b'\x07\x07\x04\x0E\x0F\x09\x07\x08\x03')
        self._write(0xE8, b'\x34')

        self._write(
            0x62,
            b'\x18\x0D\x71\xED\x70\x70\x18\x0F\x71\xEF\x70\x70')

        self._write(
            0x63,
            b'\x18\x11\x71\xF1\x70\x70\x18\x13\x71\xF3\x70\x70')

        self._write(0x64, b'\x28\x29\xF1\x01\xF1\x00\x07')
        self._write(
            0x66,
            b'\x3C\x00\xCD\x67\x45\x45\x10\x00\x00\x00')

        self._write(
            0x67,
            b'\x00\x3C\x00\x00\x00\x01\x54\x10\x32\x98')

        self._write(0x74, b'\x10\x85\x80\x00\x00\x4E\x00')
        self._write(0x98, b'\x3e\x07')
        self._write(0x35)
        self._write(0x21)
        self._write(0x11)
        time.sleep_ms(120)

        self._write(0x29)
        time.sleep_ms(20)

        self.rotation(self._rotation)

        if backlight is not None:
            backlight.value(1)

    @micropython.native
    def _write(self, command=None, data=None):
        """SPI write to the device: commands and data."""
        if self.cs:
            self.cs.off()

        if command is not None:
            self.dc.off()
            self.spi.write(bytes([command]))
        if data is not None:
            self.dc.on()
            self.spi.write(data)

        if self.cs:
            self.cs.on()

    def hard_reset(self):
        """Hard reset display."""
        if self.reset:
            if self.cs:
                self.cs.off()

            self.reset.on()
            time.sleep_ms(50)
            self.reset.off()
            time.sleep_ms(50)
            self.reset.on()
            time.sleep_ms(150)

            if self.cs:
                self.cs.on()

    def soft_reset(self):
        """Soft reset display."""
        self._write(GC9A01_SWRESET)
        time.sleep_ms(150)

    def sleep_mode(self, value):
        """
        Enable or disable display sleep mode.

        Args:
            value (bool): if True enable sleep mode.
                if False disable sleep mode
        """
        if value:
            self._write(GC9A01_SLPIN)
        else:
            self._write(GC9A01_SLPOUT)

    def inversion_mode(self, value):
        """
        Enable or disable display inversion mode.

        Args:
            value (bool): if True enable inversion mode.
                if False disable inversion mode
        """
        if value:
            self._write(GC9A01_INVON)
        else:
            self._write(GC9A01_INVOFF)

    def rotation(self, rotation):
        """
        Set display rotation.

        Args:
            rotation (int):

                - 0 - PORTRAIT
                - 1 - LANDSCAPE
                - 2 - INVERTED PORTRAIT
                - 3 - INVERTED LANDSCAPE
                - 4 - PORTRAIT MIRRORED
                - 5 - LANDSCAPE MIRRORED
                - 6 - INVERTED PORTRAIT MIRRORED
                - 7 - INVERTED LANDSCAPE MIRRORED

        """

        self._rotation = rotation % 8
        self._write(GC9A01_MADCTL, bytes([ROTATIONS[self._rotation]]))

    def _set_columns(self, start, end):
        """
        Send CASET (column address set) command to display.

        Args:
            start (int): column start address
            end (int): column end address
        """
        if start <= end <= self.width:
            self._write(GC9A01_CASET, _encode_pos(
                start, end))

    def _set_rows(self, start, end):
        """
        Send RASET (row address set) command to display.

        Args:
            start (int): row start address
            end (int): row end address
       """
        if start <= end <= self.height:
            self._write(GC9A01_RASET, _encode_pos(
                start, end))

    def _set_window(self, x0, y0, x1, y1):
        """
        Set window to column and row address.

        Args:
            x0 (int): column start address
            y0 (int): row start address
            x1 (int): column end address
            y1 (int): row end address
        """
        self._set_columns(x0, x1)
        self._set_rows(y0, y1)
        self._write(GC9A01_RAMWR)

    def vline(self, x, y, length, color):
        """
        Draw vertical line at the given location and color.

        Args:
            x (int): x coordinate
            Y (int): y coordinate
            length (int): length of line
            color (int): 565 encoded color
        """
        self.fill_rect(x, y, 1, length, color)

    def hline(self, x, y, length, color):
        """
        Draw horizontal line at the given location and color.

        Args:
            x (int): x coordinate
            Y (int): y coordinate
            length (int): length of line
            color (int): 565 encoded color
        """
        self.fill_rect(x, y, length, 1, color)

    def pixel(self, x, y, color):
        """
        Draw a pixel at the given location and color.

        Args:
            x (int): x coordinate
            Y (int): y coordinate
            color (int): 565 encoded color
        """
        self._set_window(x, y, x, y)
        self._write(None, _encode_pixel(color))

    def blit_buffer(self, buffer, x, y, width, height):
        """
        Copy buffer to display at the given location.

        Args:
            buffer (bytes): Data to copy to display
            x (int): Top left corner x coordinate
            Y (int): Top left corner y coordinate
            width (int): Width
            height (int): Height
        """
        self._set_window(x, y, x + width - 1, y + height - 1)
        self._write(None, buffer)

    def rect(self, x, y, w, h, color):
        """
        Draw a rectangle at the given location, size and color.

        Args:
            x (int): Top left corner x coordinate
            y (int): Top left corner y coordinate
            width (int): Width in pixels
            height (int): Height in pixels
            color (int): 565 encoded color
        """
        self.hline(x, y, w, color)
        self.vline(x, y, h, color)
        self.vline(x + w - 1, y, h, color)
        self.hline(x, y + h - 1, w, color)

    @micropython.native
    def fill_rect(self, x, y, width, height, color):
        """
        Draw a rectangle at the given location, size and filled with color.

        Args:
            x (int): Top left corner x coordinate
            y (int): Top left corner y coordinate
            width (int): Width in pixels
            height (int): Height in pixels
            color (int): 565 encoded color
        """
        self._set_window(x, y, x + width - 1, y + height - 1)
        chunks, rest = divmod(width * height, _BUFFER_SIZE)
        pixel = _encode_pixel(color)
        self.dc.on()
        if chunks:
            data = pixel * _BUFFER_SIZE
            for _ in range(chunks):
                self._write(None, data)
        if rest:
            self._write(None, pixel * rest)

    def fill(self, color):
        """
        Fill the entire FrameBuffer with the specified color.

        Args:
            color (int): 565 encoded color
        """
        self.fill_rect(0, 0, self.width, self.height, color)

    def line(self, x0, y0, x1, y1, color):
        """
        Draw a single pixel wide line starting at x0, y0 and ending at x1, y1.

        Args:
            x0 (int): Start point x coordinate
            y0 (int): Start point y coordinate
            x1 (int): End point x coordinate
            y1 (int): End point y coordinate
            color (int): 565 encoded color
        """
        steep = abs(y1 - y0) > abs(x1 - x0)
        if steep:
            x0, y0 = y0, x0
            x1, y1 = y1, x1
        if x0 > x1:
            x0, x1 = x1, x0
            y0, y1 = y1, y0
        dx = x1 - x0
        dy = abs(y1 - y0)
        err = dx // 2
        if y0 < y1:
            ystep = 1
        else:
            ystep = -1
        while x0 <= x1:
            if steep:
                self.pixel(y0, x0, color)
            else:
                self.pixel(x0, y0, color)
            err -= dy
            if err < 0:
                y0 += ystep
                err += dx
            x0 += 1

    def vscrdef(self, tfa, vsa, bfa):
        """
        Set Vertical Scrolling Definition.

        To scroll a 135x240 display these values should be 40, 240, 40.
        There are 40 lines above the display that are not shown followed by
        240 lines that are shown followed by 40 more lines that are not shown.
        You could write to these areas off display and scroll them into view by
        changing the TFA, VSA and BFA values.

        Args:
            tfa (int): Top Fixed Area
            vsa (int): Vertical Scrolling Area
            bfa (int): Bottom Fixed Area
        """
        struct.pack(">HHH")
        self._write(GC9A01_VSCRDEF, struct.pack(">HHH", tfa, vsa, bfa))

    def vscsad(self, vssa):
        """
        Set Vertical Scroll Start Address of RAM.

        Defines which line in the Frame Memory will be written as the first
        line after the last line of the Top Fixed Area on the display

        Example:

            for line in range(40, 280, 1):
                tft.vscsad(line)
                utime.sleep(0.01)

        Args:
            vssa (int): Vertical Scrolling Start Address

        """
        self._write(GC9A01_VSCSAD, struct.pack(">H", vssa))
