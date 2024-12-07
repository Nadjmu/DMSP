"""

# Write to SD Card from Raspberry Pi Pico
# Author: Yiming Alan Li (yimili@student.ethz.ch)
# Date: 08.11.2023
# Description: writes content to a file and then reads it

"""


import machine
import sdcard
import uos
 
# Assign chip select (CS) pin (and start it high)
cs = machine.Pin(13, machine.Pin.OUT)
 
# Intialize SPI peripheral (start with 1 MHz)
spi = machine.SPI(1,
                  baudrate=1000000,
                  polarity=0,
                  phase=0,
                  bits=8,
                  firstbit=machine.SPI.MSB,
                  sck=machine.Pin(10),
                  mosi=machine.Pin(11),
                  miso=machine.Pin(12))
 
# Initialize SD card
sd = sdcard.SDCard(spi, cs)
 
# Mount filesystem
vfs = uos.VfsFat(sd)
uos.mount(vfs, "/sd")
 
# Create a file and write something to it
with open("/sd/test02.txt", "w") as file:
    file.write("Hello, SD World!\r\n")
    file.write("i am writing from linuxxxxxx\r\n")
 
# Open the file we just created and read from it
with open("/sd/test02.txt", "r") as file:
    data = file.read()
    print(data)