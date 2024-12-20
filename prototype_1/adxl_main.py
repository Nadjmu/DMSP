"""

# ADXL345 Acceleration Sensor via I2C
# Date: 08.11.2023
# Description: initializes and reads data from the ADXL345
# https://how2electronics.com/interfacing-adxl345-accelerometer-with-raspberry-pi-pico/

"""

from machine import Pin, I2C
import time
import ustruct
 
# Constants
ADXL345_ADDRESS = 0x53
ADXL345_POWER_CTL = 0x2D
ADXL345_DATA_FORMAT = 0x31
ADXL345_DATAX0 = 0x32
 
# Initialize I2C
i2c = I2C(0, sda=Pin(20), scl=Pin(21), freq=400000)
 
# Initialize ADXL345
def init_adxl345():
    i2c.writeto_mem(ADXL345_ADDRESS, ADXL345_POWER_CTL, bytearray([0x08]))  # Set bit 3 to 1 to enable measurement mode
    i2c.writeto_mem(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, bytearray([0x0B]))  # Set data format to full resolution, +/- 16g
 
# Read acceleration data
def read_accel_data():
    data = i2c.readfrom_mem(ADXL345_ADDRESS, ADXL345_DATAX0, 6)
    x, y, z = ustruct.unpack('<3h', data)
    return x, y, z
 
# Main loop
init_adxl345()
while True:
    x, y, z = read_accel_data()
    print("X: {}, Y: {}, Z: {}".format(x, y, z))
    time.sleep(0.1)