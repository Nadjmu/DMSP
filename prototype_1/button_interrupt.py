"""
A button is connected to GPIO17 with internal Pull-up
A LED is connected to GPIO16
Debouncing is solved using a software approach with a timer of 500ms
LED is toggled when button is clicked
"""

from machine import Pin
import time 

interrupt_flag = 0
debounce_time = 0
counter = 0
led = Pin(16,Pin.OUT)
pin = Pin(17,Pin.IN,Pin.PULL_UP)

def callback(pin):
    global interrupt_flag, debounce_time
    if (time.ticks_ms()-debounce_time)>500:
        interrupt_flag=1
        debounce_time=time.ticks_ms()
        print("debounce_time={}".format(debounce_time))

pin.irq(trigger=Pin.IRQ_RISING, handler=callback)
while True:
    if interrupt_flag is 1:
        print("Interrupt has occured")
        interrupt_flag=0
        counter+=1
        led.toggle()
        print("Count={}".format(counter))