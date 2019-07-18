# hab-tdr
## Introduction
Welcome to hab-tdr repository. This is the place of the internet where I am going to store the source code of the high altitude balloon I'm building with Arduino.
Code's comments are written in spanish as it is my native language. Nevertheless, the code is not that difficult to understand without them.

This project is my Treball de Recerca (TdR), a common project which has to be done as school work on baccalaureate in Catalonia.

Fully written in C++, this WIP hab gets sensor data about temperature, humidity, pressure and height and stores them on a SD card, as well as it sends them to the specified server on the ESP8266.

## Source code

### Communication
Arduino-Arduino communication is done with Serial communication. Data is sent from the Arduino which the sensors are connected at to the Arduino with the GPS, the SD card and the Wifi module connected to.

Data is sent as the following format:
$TEMPERATURE,HUMIDITY,PREASSURE,HEIGHT*

Arduino-ESP8266 communication is done with Serial communication as well. Data is sent from the Arduino with the GPS/SD Card to the ESP8266.
Data is sent as the following format:
TEMPERATURE,HUMIDITY,PREASSURE,HEIGHT

## Folders
### cpu_
Every folder whose name starts with 'cpu' contains the code for each MCU.

### test_
Every folder whose name starts with 'test', contains the code used to test some function of the hab in the past.

## Author
Adrián Escalona
Twitter: @a7escalona
E-mail: adri7escalona@gmail.com
