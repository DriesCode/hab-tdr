#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <Wire.h>

DHT sensorDHT (2, DHT22); // Creamos una instancia de la clase DHT
Adafruit_BMP085 sensorBMP; // Creamos una instancia de la clase Adafruit_BMP085

int mode = 0;  /*
                  0 = Temperatura
                  1 = Humedad
                  2 = Presion
                  3 = Altitud
              */


void setup() {
  sensorDHT.begin();
  sensorBMP.begin();
  Serial.begin(9600);
}

void loop() {
  mode++;
  if (mode > 3) mode = 0;
  
  delay(2000);

  float humedad = sensorDHT.readHumidity();
  float temperatura = sensorDHT.readTemperature();
  float presion = sensorBMP.readPressure();
  float altitud = sensorBMP.readAltitude(101500);
 
  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) {
    Serial.println("Error de los sensores");
    return; 
  }

  switch (mode) {
    case 0:
      Serial.println(temperatura);
     break;
     case 1: 
       Serial.println(humedad);
      break;
     case 2:
       Serial.println(presion);
     break;
     case 3:
       Serial.println(altitud);
     break;
  }
}
