#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#define RX_PIN 11 // Pin recibir datos en Arduino ( No usado )
#define TX_PIN 10 // Pin enviar datos en Arduino

SoftwareSerial arduino2 (RX_PIN, TX_PIN); // Conexión con el arduino
DHT sensorDHT (2, DHT22); // Conexión con el sensor de temperatura y humedad
Adafruit_BMP085 sensorBMP; // Conexión con el sensor de presión y altitud
LiquidCrystal lcd(8,9,4,5,6,7); // Conexión con el LCD

long slpresion = 102000; // Variable para almacenar la presión actual al nivel del mar

float humedad, temperatura, presion, altitud; // Variables que almacenarán los valores de los sensores

void setup() {
  // Iniciar la comunicación con los dispositivos
  arduino2.begin(9600); 
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16,2); // Iniciar la LCD y especificar filas y columnas

  Serial.begin(9600); // Iniciar comunicación con el ordenador

  // Imprimir información sobre el programa
  printLcd(&lcd, "Arduino Core");
  delay(3000);
  printLcd(&lcd, "Altimetro: " + (String) slpresion + " Pa");
  delay(2500);

}

void loop() {

  // Leer valores de los sensores
  humedad = sensorDHT.readHumidity();
  temperatura = sensorDHT.readTemperature();
  presion = sensorBMP.readPressure();
  altitud = sensorBMP.readAltitude(slpresion);

  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) { // Si algún valor es nulo, no continuar el programa
      printLcd(&lcd, "Error de los sensores");
      while (1) {;}
  }

  printLcd(&lcd, "Datos captados"); // Mostrar que los datos ya se han recogido
  delay(1500);

  printLcd(&lcd, "Enviando"); // Mostrar que se van a enviar los datos

  // FORMATO DATOS: TEMPERATURA,HUMEDAD,PRESION,ALTITUD,0
  String cadenaDatos = String((String) temperatura + "," + (String) humedad + "," + (String) presion + "," + (String) altitud + "*"); // Construir la cadena de datos

  arduino2.println(cadenaDatos); // Enviar los datos por serial al otro arduino
  delay(1500);

}

// Rutina para imprimir texto en la LCD
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
