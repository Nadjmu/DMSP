"""

# Minimalistic Prototype 1
# Author: Yiming Alan Li (yimili@student.ethz.ch)
# Date: 08.11.2023
# Description: collects GPS data and write them to a csv file on a sd card

    data string has the following format:
    Date [mm/dd/yyyy], Time [HH:MM:SS.ZZ], Latitude [deg], Longitude [deg]

"""
from machine import Pin, UART
import sdcard
import uos
import utime
from micropyGPS import MicropyGPS

import utime_date

# Assign chip select (CS) pin (and start it high)
cs = Pin(13, Pin.OUT)
led = Pin(16,Pin.OUT)
# Intialize SPI peripheral (start with 1 MHz)
spi = machine.SPI(1,
                  baudrate=1000000,
                  polarity=0,
                  phase=0,
                  bits=8,
                  firstbit=machine.SPI.MSB,
                  sck=Pin(10),
                  mosi=Pin(11),
                  miso=Pin(12))
 
# Initialize SD card
sd = sdcard.SDCard(spi, cs)
 
# Mount filesystem
vfs = uos.VfsFat(sd)
uos.mount(vfs, "/sd")
 

# Initialize GPS module
gps_module = UART(1, baudrate=9600, tx=Pin(4), rx=Pin(5))
time_zone = +1
gps = MicropyGPS(time_zone)

def convert_coordinates(sections):
    if sections[0] == 0:  # sections[0] contains the degrees
        return None

    # sections[1] contains the minutes
    data = sections[0] + (sections[1] / 60.0)

    # sections[2] contains 'E', 'W', 'N', 'S'
    if sections[2] == 'S':
        data = -data
    if sections[2] == 'W':
        data = -data

    data = '{0:.6f}'.format(data)  # 6 decimal places
    return str(data)


with open("/sd/exp01.csv","w") as file:
    file.write("Date [mm/dd/yyyy], Time [HH:MM:SS.ZZ], Latitude [deg], Longitude [deg]\r\n")

    
while True:
    length = gps_module.any()
    if length > 0:
        data = gps_module.read(length)
        for byte in data:
            message = gps.update(chr(byte))

    latitude = convert_coordinates(gps.latitude)
    longitude = convert_coordinates(gps.longitude)
    if latitude is None or longitude is None:
        continue
    #print('Lat: ' + latitude)
    #print('Lon: ' + longitude)
    
    data = ""
    year, month, day, hour, minute, second, centisecond = utime_date.get_datetime_details()
    data += str(month)+"/"+str(day)+"/"+str(year)+","
    data += str(hour)+":"+str(minute)+":"+str(second)+"."+str(centisecond)+","
    data += str(latitude)+","
    data += str(longitude)+"\r\n"
    
    #writing to the SD file
    with open("/sd/exp01.csv","a") as file:
        file.write(data)
    utime.sleep_ms(100)  # Add a 100ms delay between GPS updates
