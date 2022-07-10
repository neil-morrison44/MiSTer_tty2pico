from machine import SPI, Pin
import gc
import array
from png import Reader
from sys import stdin
from gc9a01py import GC9A01

gc.enable()

DISPLAY_WIDTH = const(240)
DISPLAY_HEIGHT = const(240)

RED_MASK = const(0b11111000)
GREEN_MASK = const(0b11111100)

BLIT_MEMORY_THRESHOLD = const(9000)
BYE_MESSAGE = const("bye")
ARRAY_TYPECODE = const("h")

tft_clk = Pin(10, Pin.OUT)  # must be a SPI CLK
tft_mosi = Pin(11, Pin.OUT)  # must be a SPI TX
tft_rst = Pin(12, Pin.OUT)
tft_dc = Pin(13, Pin.OUT)
tft_cs = Pin(14, Pin.OUT)
tft_bl = Pin(15, Pin.OUT)

spi = SPI(1, sck=tft_clk, mosi=tft_mosi)
display = GC9A01(spi, reset=tft_rst, dc=tft_dc, cs=tft_cs, backlight=tft_bl)

current_logo = "none"
display_asleep = False


def display_logo(core_name):
    file_name = "/logos/" + core_name + ".png"
    print(file_name)
    global current_logo
    if (current_logo == file_name):
        return
    current_logo = file_name
    r = Reader(filename=file_name)
    width, height, rows, info = r.read()
    greyscale = info["greyscale"]
    gc.collect()
    x_offset = round((DISPLAY_WIDTH / 2) - (width / 2))
    y_offset = round((DISPLAY_HEIGHT / 2) - (height / 2))
    y = 0
    y_blit_start = 0

    lineBytes = array.array(ARRAY_TYPECODE)
    for row in rows:
        if (gc.mem_free() < BLIT_MEMORY_THRESHOLD or y == height - 1):
            display.blit_buffer(lineBytes, x_offset, y_offset +
                                y_blit_start, width, y - y_blit_start)
            lineBytes = array.array(ARRAY_TYPECODE)
            y_blit_start = y
        x = 0
#         lineBytes = array.array("h")
        while x < width:
            r = 0
            g = 0
            b = 0
            if (greyscale):
                r = b = g = row[x]
            else:
                r = row[(x * 3)]
                g = row[(x * 3) + 1]
                b = row[(x * 3) + 2]
            rgb565 = ((r & RED_MASK) << 8) | ((g & GREEN_MASK) << 3) | (b >> 3)
            if (x == 0 and y == 0):
                display.fill(rgb565)
            swapped565 = (rgb565 >> 8) | (rgb565 << 8)
            lineBytes.append(swapped565)
            x += 1
#         display.blit_buffer(lineBytes, x_offset, y_offset+y, width, 1)
        y += 1
    gc.collect()


def wake_display():
    global display_asleep
    if (display_asleep):
        display.sleep_mode(False)
        tft_bl.value(1)
        display_asleep = False


def sleep_display():
    global display_asleep
    if (not display_asleep):
        display.sleep_mode(True)
        tft_bl.value(0)
        display_asleep = True


def read_from_input():
    while True:
        input = stdin.readline()
        clean_input = input.strip()
        print(clean_input)
        if (len(clean_input) == 0):
            continue
        if (clean_input == BYE_MESSAGE):
            sleep_display()
            continue
        else:
            wake_display()
        try:
            display_logo(clean_input)
        except OSError:
            print("no logo!")
        except MemoryError:
            print("logo too big!")


# display_logo("mister")
# read_from_input()
