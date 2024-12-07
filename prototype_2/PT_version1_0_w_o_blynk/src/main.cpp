/*
Date: 22.03.2024
Author: Yiming Alan Li 
*/

#include <Arduino.h>
#include <stdio.h>
#include <PL_ADXL355.h>
#include "FS.h"
#include "SD_MMC.h"

#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));



#define ONE_BIT_MODE true

// Create an instance of ADXL355
// ADXL355 MOSI, MISO and SCLK pins should be connected to the correspondent
// Arduino pins and ADXL355 CS pin should be connected to pin 2
PL::ADXL355 adxl355(5);

// ADXL355 range: +/- 2 g
auto range = PL::ADXL355_Range::range2g;

typedef struct{
  float x; 
  float y;
  float z;
  unsigned long time;
} AccelerationValues;

struct GpsValues {
    double latitude_deg;
    double longitude_deg; 
    unsigned long time;

    // Default constructor
    GpsValues() 
        : latitude_deg(90.0), // Default value for latitude
          longitude_deg(0.0), // Default value for longitude
          time(0) { // Default value for time
        // Constructor body (if needed)
    }
};

//==============================================================================

#define BUFFER_SIZE_ACC 200
#define BUFFER_SIZE_GPS 5

AccelerationValues acc_buffer[2*BUFFER_SIZE_ACC];
GpsValues gps_buffer[2*BUFFER_SIZE_GPS];

int k = 0;

int generate_next_sample(void *parameter);
void printArray(int array[], int size, int start);
AccelerationValues getAccelerationValues();
GpsValues getGpsValues();
///other method with notification
TaskHandle_t taskHandle1;
TaskHandle_t taskHandle2;


// Settings
static const uint16_t timer_divider = 80; // Count at 1 MHz
static const uint64_t timer_max_count = 10000;
static const uint64_t timer_max_count1 = 1000000;

// Globals
static hw_timer_t *timer_acc = NULL;
static hw_timer_t *timer_gps = NULL; 
static SemaphoreHandle_t bin_sem_acc = NULL;
static SemaphoreHandle_t bin_sem_gps = NULL; 
static volatile bool prev_timer_acc_state = false; 
static volatile bool curr_timer_acc_state = false; 
static volatile bool prev_timer_gps_state = false; 
static volatile bool curr_timer_gps_state = false; 

//read SD card content
static volatile bool read_sd = false;

//////////////////// ACC ////////////////////////////////////////

AccelerationValues getAccelerationValues(){
  auto accelerations = adxl355.getAccelerations();
  unsigned long time = millis();
  AccelerationValues acc; 
  acc.x = accelerations.x;
  acc.y = accelerations.y;
  acc.z = accelerations.z;
  acc.time = time;
  
  return acc;
}

GpsValues getGpsValues(){
  myGNSS.checkUblox();
  if (nmea.isValid()){
    GpsValues gps;
    gps.latitude_deg = nmea.getLatitude()/1000000.;
    gps.longitude_deg = nmea.getLongitude()/1000000.;
    gps.time = millis();
    nmea.clear();
    return gps;
  }
  else {
    return GpsValues();
  }
}

//*****************************************************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimerAcc() {

  BaseType_t task_woken = pdFALSE;

  // Give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(bin_sem_acc, &task_woken);

  // Exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}


// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimerGps() {

  BaseType_t task_woken = pdFALSE;

  // Give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(bin_sem_gps, &task_woken);

  // Exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

/////////////////////////////////SD//////////////////////////////////

void appendSingleAccToSD(fs::FS &fs, const char * path, AccelerationValues acc[], int length, int start){
  File file = fs.open(path, FILE_APPEND);
  if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
  for (int i = start; i < start+length; i++){
    AccelerationValues current_acc;
    current_acc.x = acc[i].x;
    current_acc.y = acc[i].y;
    current_acc.z = acc[i].z;
    current_acc.time = acc[i].time;
    char message[100]; // Define a char array with sufficient size
    snprintf(message, sizeof(message), "t=%lu x=%f, y=%f, z=%f \n",current_acc.time, current_acc.x, current_acc.y, current_acc.z);
    file.print(message);
  }
  file.close();
}

void appendSingleGpsToSD(fs::FS &fs, const char * path, GpsValues gps[], int length, int start){
  File file = fs.open(path, FILE_APPEND);
  if (!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  for (int i = start; i < start+length; i++){
    GpsValues current_gps;
    current_gps.latitude_deg = gps[i].latitude_deg;
    current_gps.longitude_deg = gps[i].longitude_deg;
    current_gps.time = gps[i].time;
    char message[100];
    snprintf(message, sizeof(message), "t=%lu lat=%f, long=%f \n",current_gps.time, current_gps.latitude_deg, current_gps.longitude_deg);
    file.print(message);
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
/////////////////////////////////////////////////////////////////////////
/////////TASKS/////////
/// Task 1 runs on core 0

void writeAccBuffer(void *parameter){
  int buffer_index = 0; 
  uint32_t notifyValue = 1;
  while(1){
    if (buffer_index == (BUFFER_SIZE_ACC)){
      Serial.println("finished writing the first acc buffer ");
      notifyValue = 1; 
      Serial.println("Sending notification to AccToSD with value 1");
      xTaskNotify(taskHandle1, notifyValue, eSetValueWithoutOverwrite);
    }
    else if (buffer_index == (2*BUFFER_SIZE_ACC)){
      Serial.println("finished wriitng the second acc buffer");
      notifyValue = 2; 
      Serial.println("Sending notification to AccToSD with value 2");
      xTaskNotify(taskHandle1, notifyValue, eSetValueWithoutOverwrite);
      buffer_index = 0;
      k = 0; 
    }
    if (xSemaphoreTake(bin_sem_acc, portMAX_DELAY)){
      acc_buffer[buffer_index] = getAccelerationValues();
      buffer_index++;
    }
  }
}

void writeGpsBuffer(void *parameter){
  int buffer_index = 0;
  uint32_t notifyValue = 1; 
  while(1){
    if (buffer_index == (BUFFER_SIZE_GPS)){
      Serial.println("finished writing the first gps buffer");
      notifyValue = 1; 
      Serial.println("Sending notification to GpsToSD with value 1");
      xTaskNotify(taskHandle2, notifyValue, eSetValueWithoutOverwrite);
    }
    else if (buffer_index == (2*BUFFER_SIZE_GPS)){
      Serial.println("finished writing the second gps buffer");
      notifyValue = 2; 
      Serial.println("Sending notification to GpsToSd with value 2");
      xTaskNotify(taskHandle2, notifyValue, eSetValueWithoutOverwrite);
      buffer_index = 0; 
    }
    if (xSemaphoreTake(bin_sem_gps, portMAX_DELAY)){
      gps_buffer[buffer_index] = getGpsValues();
      buffer_index++;
    }
  }
}

//Task 2 runs on core 1

void AccToSD(void *parameter){
  uint32_t receivedValue; 
  unsigned long timeSinceStart;
  unsigned long timeSinceEnd;
  while(1){
    if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &receivedValue, portMAX_DELAY) == pdTRUE) {
      if (receivedValue == 1) {
          Serial.println("received notification with value 1");
          timeSinceStart = millis();
          appendFile(SD_MMC, "/imp_exp.txt", "Block 1 ACC \n");
          appendSingleAccToSD(SD_MMC,"/imp_exp.txt", acc_buffer, BUFFER_SIZE_ACC, (receivedValue-1)*BUFFER_SIZE_ACC);
          timeSinceEnd = millis();
          Serial.print("time it took to read the first buffer in ms:  ");Serial.println(timeSinceEnd-timeSinceStart);
        }
      else if (receivedValue == 2) {
          Serial.println("received notification with value 2");
          timeSinceStart = millis();
          appendFile(SD_MMC, "/imp_exp.txt", "Block 2 \n");
          appendSingleAccToSD(SD_MMC,"/imp_exp.txt", acc_buffer, BUFFER_SIZE_ACC, (receivedValue-1)*BUFFER_SIZE_ACC);
          timeSinceEnd = millis();
          Serial.print("time it took to read the second buffer in ms:  ");Serial.println(timeSinceEnd-timeSinceStart);
        }
    }
  }
}

void GpsToSD(void *parameter){
  uint32_t receivedValue; 
  unsigned long timeSinceStart;
  unsigned long timeSinceEnd; 
  while(1){
    if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &receivedValue, portMAX_DELAY) == pdTRUE){
      if (receivedValue == 1){
        Serial.println("received notification with value 1 from gps");
        timeSinceStart = millis();
        appendFile(SD_MMC, "/imp_exp.txt", "Block 1 GPS \n");
        appendSingleGpsToSD(SD_MMC,"/imp_exp.txt", gps_buffer, BUFFER_SIZE_GPS, (receivedValue-1)*BUFFER_SIZE_GPS);
        timeSinceEnd = millis();
        Serial.print("time it took to read the first gps buffer in ms:  "); Serial.println(timeSinceEnd-timeSinceStart);
      }
      else if (receivedValue == 2){
        Serial.println("received notification with value 2 from gps");
        timeSinceStart = millis();
        appendFile(SD_MMC, "/imp_exp.txt", "Block 2 GPS \n");
        appendSingleGpsToSD(SD_MMC,"/imp_exp.txt", gps_buffer, BUFFER_SIZE_GPS, (receivedValue-1)*BUFFER_SIZE_GPS);
        timeSinceEnd = millis();
        Serial.print("time it took to read the second gps buffer in ms:  "); Serial.println(timeSinceEnd-timeSinceStart);
      }
    }

  }
}


void check_read_SD(void *parameter){
  Serial.println("created check read SD Task");
  while(1){
    if(read_sd){
      readFile(SD_MMC, "/imp_exp.txt");
      read_sd = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  //arraySize = sizeof(acc_buffer) / sizeof(acc_buffer[0]); 
  //Serial.println(arraySize);

  ////////////////////////////////////  INIT GPS ////////////////////////////////////
  Wire.begin();

  if (myGNSS.begin() == false)
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  //myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_ALL); // Make sure the library is passing all NMEA messages to processNMEA

  myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_GGA); // Or, we can be kind to MicroNMEA and _only_ pass the GGA messages to it

  //////////////// INIT SD ////////////////
    if(!SD_MMC.begin("/sdcard",ONE_BIT_MODE)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return;
    }

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  writeFile(SD_MMC, "/imp_exp.txt", "This is an important test \n ");

  ////////////////// INIT ACC ///////////////
  adxl355.begin();
  adxl355.setRange(range);
  adxl355.enableMeasurement();

  vTaskDelay(500 / portTICK_PERIOD_MS);

  ///////////////// INIT TIMER ///////////////
  timer_acc = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer_acc, &onTimerAcc, true);
  timerAlarmWrite(timer_acc, timer_max_count, true);
  timerAlarmEnable(timer_acc);

  timer_gps = timerBegin(1, timer_divider, true);
  timerAttachInterrupt(timer_gps, &onTimerGps, true);
  timerAlarmWrite(timer_gps, timer_max_count1, true);
  timerAlarmEnable(timer_gps);

  bin_sem_acc = xSemaphoreCreateBinary();
  bin_sem_gps = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(writeAccBuffer, "writeAccBuffer", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(writeGpsBuffer, "writeGpsBuffer", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(AccToSD, "AccToSD", 10000, NULL, 3, &taskHandle1, 1);
  xTaskCreatePinnedToCore(GpsToSD, "GpsToSD", 10000, NULL, 3, &taskHandle2, 1);
  xTaskCreatePinnedToCore(check_read_SD, "check_read_SD", 8000, NULL, 1, NULL, 1);
  //xTaskCreatePinnedToCore(printValues, "printValues", 1000, NULL, 1, NULL, 0);
  //printArray(acc_buffer, arraySize);

  // Start timer 
  //timerAlarmEnable(timer_acc);

  vTaskDelay(500 / portTICK_PERIOD_MS);
}


void loop() {
  // put your main code here, to run repeatedly:
}


//This function gets called from the SparkFun u-blox Arduino Library
//As each NMEA character comes in you can specify what to do with it
//Useful for passing to other libraries like tinyGPS, MicroNMEA, or even
//a buffer, radio, etc.
void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
  //Take the incoming char from the u-blox I2C port and pass it on to the MicroNMEA lib
  //for sentence cracking
  nmea.process(incoming);
}