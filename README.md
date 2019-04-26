# hab-tdr
## Introduction
Welcome to hab-tdr repository. This is the place of the internet where I am going to store the source code of the high altitude balloon I'm building with Arduino.
Code's comments are written in spanish as it is my native language. Nevertheless, the code is not that difficult to understand without them.

This project is my Treball de Recerca (TdR), a common project which has to be done as school work on baccalaureate in Catalonia.

## Source code

### Communication
Arduino-communication is done with Serial communication. Data is sent from the Arduino which the sensors are connected at to the Arduino with the GPS, the SD card and the GSM module connected to.

Data is sent as the following format:
$TEMPERATURE,HUMIDITY,PREASSURE,ALTITUDE*

## Author
Adrián Escalona
Twitter: @a7escalona
E-mail: adri7escalona@gmail.com
