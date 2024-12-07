# DMSP
This git-repository is a collection of components and prototypes for the Dynamic Mobile Sensing Platform. The primary goal for this project specifically is to develop a device capable of collecting GNSS-positioning data as well as acceleration data along all axis. The data is transfered via the MQTT-messaging protocol. For more information, refer to [Dynamic Mobile Sensing Platform](https://frs.ethz.ch/research/frs-phase-ii--2020-2013/distributed-cognition/dynamic-mobile-sensing-platform.html).

## Components
A brief overview of all the components required for this project

### Microcontroller
Raspberry Pi Pico    
Esp32 DevKitC

### Acceleration Sensor
ADXL 345  
ADXL 355    
ADXL 367

### GPS 
Neo-6M   
Neo-9M

### SD Card 
SDMMC-host on ESP32 with external SD-module

### IMU
ICM20948 9-DoF IMU 


## Development frameworks used
- PlatformIO 
- Micropython


## Features
- [**FreeRTOS**](https://www.freertos.org/index.html): a real-time operating system for microcontrollers to run multitask-, time-critical and resource constrained applications
- **Deep-Sleep** for controlled and reduced power consumption    
- [**TensorFlowLite**](https://www.tensorflow.org/lite/microcontrollers) for Microcontrollers to run Deep-Learning Models on the processor (Edge AI)
- [**Blynk**](https://blynk.io/): an IoT-Platform with full back-end infrastructure for features such as Mobile App, Web Dashboard, Device Provisioning, Remote Control, Over-The-Air firmware updates, etc...


## Prototype 1
- **Development framework:** Micropython
- This prototype is used for fast experimenting and proof of concepts due to the easy implementation Micropython offers
### Components and their Peripherals
- Microcontroller: Raspberry Pi Pico
- ADXL345 via I2C0
- NEO6M via UART1
- SIM7080G via UART0
- SD via SPI1

## Prototype 2
- **Development framework:** PlatformIO
### Components and their Peripherals
- Microcontroller: ESP32 DevKitC
- ADXL355 via SPI
- NEO9M over I2C
- SD over SDMMC
- ICM20948 over I2C

## More Ideas
Here you can commit your ideas for the project: 

