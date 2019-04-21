#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#include <Wire.h>

/*#define RX_PIN 10
#define TX_PIN 11*/

//SoftwareSerial arduino2 (RX_PIN, TX_PIN);
DHT sensorDHT (2, DHT22);
Adafruit_BMP085 sensorBMP;
LiquidCrystal lcd(8,9,4,5,6,7);

long slpresion = 102000;

void setup() {
 // arduino2.begin(9600);
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16,2);

  //arduino2.print("Test");

  Wire.begin();
  Wire.beginTransmission(9);
  Wire.write("Test");
  Wire.endTransmission();

  printLcd(&lcd, "Arduino Core");

  delay(3000);

  printLcd(&lcd, "Altimetro: " + (String) slpresion + " Pa");

  delay(1500);

}

void loop() {
  lcd.clear();

  float humedad = sensorDHT.readHumidity();
  float temperatura = sensorDHT.readTemperature();
  float presion = sensorBMP.readPressure();
  float altitud = sensorBMP.readAltitude(slpresion);

  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) {
      printLcd(&lcd, "Error de los sensores");
      while (1) {;}
  }

  printLcd(&lcd, "Datos captados");
  delay (1500);

  printLcd(&lcd, "Enviando");
  

  // FORMATO DATOS: $TEMPERATURA,HUMEDAD,PRESION,ALTITUD*
  String cadenaDatos = String((String) temperatura + "," + (String) humedad + "," + (String) presion + "," + (String) altitud + ",0");

  int i;
  for (i = 0; i < cadenaDatos.length(); i++) {
    char c = cadenaDatos.charAt(i);
    //arduino2.write(c);
    
  }

  Wire.begin();
  Wire.beginTransmission(9);
  Wire.write("Test");
  Wire.endTransmission();

  delay(1500);

}

void printLcd(LiquidCrystal* lcd, String text) {
  lcd->clear();
  if (text.length() > 16) {
    for (int i = 0; i < text.length(); i++) {
      if (i == 16) {
        lcd->setCursor(0, 1);
      }
      lcd->print(text.charAt(i));
    }
  } else {
    lcd->print(text);
  }
}
