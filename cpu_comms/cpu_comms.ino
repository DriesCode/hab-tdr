
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Wire.h>

#define RX_PIN 7
#define TX_PIN 8

#define GPS_RX_PIN 3
#define GPS_TX_PIN 4

File logs;
SoftwareSerial arduino1 (RX_PIN, TX_PIN);
SoftwareSerial ssGPS (GPS_RX_PIN, GPS_TX_PIN);
TinyGPSPlus gps;

String zero = "0";

double lati, longi;
float temperatura, presion, humedad, altitud;
uint8_t hora, minuto, segundo;
String horaStr, minutoStr, segundoStr, tiempo, latiStr, longiStr;

String datos;

void setup() {
  Serial.begin(9600);
  arduino1.begin(9600);
  ssGPS.begin(9600);
  

  //while (!arduino1) { ; }
  //Serial.println("arduino 1 conectado");
  //if (!SD.begin()) { Serial.println("Error iniciando la tarjeta SD"); while (1) { ; } }
}

void loop() {
  datos = "";
  arduino1.listen();

  if (arduino1.isListening()) {
    Serial.println("Listenign arduino");
    while (arduino1.available() > 0) {
      char c = arduino1.read();
      if (c != "-1") datos += c;
  }

    delay(1500);
  
    Serial.println(datos);
  }
  
  

  ssGPS.listen();

  if (ssGPS.isListening()) {
    Serial.println("listening gps");
    while (ssGPS.available() > 0) gps.encode(ssGPS.read());

    if (!gps.time.isUpdated()) return;
    
    getTime(&gps);
    
    Serial.println(tiempo);
    
    delay(1500);
  }

  
  
}

void getArduinoSensorData(SoftwareSerial* arduino1) {
  if (!arduino1->available() > 0) return;

  Serial.println(arduino1->read());
}

void getLocation(TinyGPSPlus* gps) {
  lati = gps->location.lat();
  longi = gps->location.lng();
}

void getTime(TinyGPSPlus* gps) {
  hora = gps->time.hour();
  minuto = gps->time.minute();
  segundo = gps->time.second();

  horaStr = ((hora < 10 && hora != 0) ? zero += hora : String(hora));
  zero = "0";
  minutoStr = ((minuto < 10 && minuto != 0) ? zero += minuto : String(minuto));
  zero = "0";
  segundoStr = ((segundo < 10 && segundo != 0) ? zero += segundo : String(segundo));
  zero = "0";

  horaStr = ((hora == 0) ? String("00") : String(horaStr));
  minutoStr = ((minuto == 0) ? String("00") : String(minutoStr));
  segundoStr = ((segundo == 0) ? String("00") : String(segundoStr));

  tiempo = String("[" + horaStr + ":" + minutoStr + ":" + segundoStr + "] ");
}
