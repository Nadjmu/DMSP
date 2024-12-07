"""
A button is connected to GPIO 17 with internal Pull-up. For every time the button is clicked, a counter is
increased by 1.
To deal with debouncing a conditional statement to the increase is added where you can only increase the counter
every 300ms. 

"""

from machine import Pin
import time

debounce_time = 0
counter = 0
pin = Pin(17, Pin.IN, Pin.PULL_UP)

while True:
    if ((pin.value()==0) and (time.ticks_ms()-debounce_time > 300)):
        print("Button Pressed")
        counter+=1
        debounce_time = time.ticks_ms()
        print("Count={}".format(counter))