#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

File gpsFile;
SoftwareSerial gps(4,3);
TinyGPSPlus tGPS;

void setup() {
  Serial.begin(9600);
  gps.begin(9600);

  while (!Serial) { ; }

  SD.begin();
}

void loop() {
  while (gps.available()) {
    tGPS.encode(gps.read());
    if (tGPS.location.isUpdated()) {
      Serial.print("Latitude= "); 
      Serial.print(tGPS.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(tGPS.location.lng(), 6);
    }
  }
}
