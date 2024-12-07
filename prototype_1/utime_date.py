"""

# Author: Yiming Alan Li (yimili@student.ethz.ch)
# Date: 08.11.2023
# Description: Function to get current date and time - should later be replaced with the time data encoded in NMEA sentences

"""


import utime

def get_datetime_details():
    # Get the current date and time
    current_time = utime.localtime()
    
    # Extract the desired components
    year = current_time[0]
    month = current_time[1]
    day = current_time[2]
    hour = current_time[3]
    minute = current_time[4]
    second = current_time[5]
    
    # Get the current microsecond tick count and derive centiseconds
    centisecond = (utime.ticks_us() % 1000000) // 10000
    
    return year, month, day, hour, minute, second, centisecond

# Test the function
"""
year, month, day, hour, minute, second, centisecond = get_datetime_details()
print("Year:", year)
print("Month:", month)
print("Day:", day)
print("Hour:", hour)
print("Minute:", minute)
print("Second:", second)
print("Centisecond:", centisecond)
"""