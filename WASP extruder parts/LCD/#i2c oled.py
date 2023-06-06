#i2c oled
from machine import Pin, I2C
import ssd1306

# using default address 0x3C
i2c = I2C(sda=Pin(4), scl=Pin(5))
display = ssd1306.SSD1306_I2C(128, 64, i2c)
Print Hello World on the first line:

display.text('Hello, World!', 0, 0, 1)
display.show()
#Basic functions:

display.poweroff()     # power off the display, pixels persist in memory
display.poweron()      # power on the display, pixels redrawn
display.contrast(0)    # dim
display.contrast(255)  # bright
display.invert(1)      # display inverted
display.invert(0)      # display normal
display.rotate(True)   # rotate 180 degrees
display.rotate(False)  # rotate 0 degrees
display.show()         # write the contents of the FrameBuffer to display memory
#Subclassing FrameBuffer provides support for graphics primitives:

display.fill(0)                         # fill entire screen with colour=0
display.pixel(0, 10)                    # get pixel at x=0, y=10
display.pixel(0, 10, 1)                 # set pixel at x=0, y=10 to colour=1
display.hline(0, 8, 4, 1)               # draw horizontal line x=0, y=8, width=4, colour=1
display.vline(0, 8, 4, 1)               # draw vertical line x=0, y=8, height=4, colour=1
display.line(0, 0, 127, 63, 1)          # draw a line from 0,0 to 127,63
display.rect(10, 10, 107, 43, 1)        # draw a rectangle outline 10,10 to 117,53, colour=1
display.fill_rect(10, 10, 107, 43, 1)   # draw a solid rectangle 10,10 to 117,53, colour=1
display.text('Hello World', 0, 0, 1)    # draw some text at x=0, y=0, colour=1
display.scroll(20, 0)                   # scroll 20 pixels to the right

# draw another FrameBuffer on top of the current one at the given coordinates
import framebuf
fbuf = framebuf.FrameBuffer(bytearray(8 * 8 * 1), 8, 8, framebuf.MONO_VLSB)
fbuf.line(0, 0, 7, 7, 1)
display.blit(fbuf, 10, 10, 0)           # draw on top at x=10, y=10, key=0
display.show()