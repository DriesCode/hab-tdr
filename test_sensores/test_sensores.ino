#include <LiquidCrystal.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <Wire.h>

DHT sensorDHT (2, DHT22); // Creamos una instancia de la clase DHT
Adafruit_BMP085 sensorBMP; // Creamos una instancia de la clase Adafruit_BMP085
LiquidCrystal lcd(8,9,4,5,6,7);

int mode = 0;  /*
                  0 = Temperatura
                  1 = Humedad
                  2 = Presion
                  3 = Altitud
              */

void setup() {
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16, 2);  
}

void loop() {
  mode++;
  if (mode > 3) mode = 0;
  
  delay(2000);
  
  lcd.clear();

  float humedad = sensorDHT.readHumidity();
  float temperatura = sensorDHT.readTemperature();
  float presion = sensorBMP.readPressure();
  float altitud = sensorBMP.readAltitude(101500);
 
  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) {
    lcd.print("Error de los sensores");
    return; 
  }

  switch (mode) {
    case 0:
      lcd.print("Temperatura:");
      lcd.setCursor(0,1);
      lcd.print(temperatura); lcd.print(" C");
     break;
     case 1: 
       lcd.print("Humedad:");
       lcd.setCursor(0,1);
       lcd.print(humedad); lcd.print(" %");
      break;
     case 2:
       lcd.print("Presion:");
       lcd.setCursor(0,1);
       lcd.print(presion); lcd.print(" Pa");
     break;
     case 3:
       lcd.print("Altitud:");
       lcd.setCursor(0,1);
       lcd.print(altitud); lcd.print(" m");
     break;
  }
}
