#include <SoftwareSerial.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal.h>

#define RX_PIN 11 // Pin recibir datos en Arduino ( No usado )
#define TX_PIN 10 // Pin enviar datos en Arduino

#define LEDCOMMS 3
#define ERRORPIN 12

SoftwareSerial arduino2 (RX_PIN, TX_PIN); // Conexión con el arduino
DHT sensorDHT (2, DHT22); // Conexión con el sensor de temperatura y humedad
SFE_BMP180 sensorBMP; // Conexión con el sensor de presión y altitud
LiquidCrystal lcd(8,9,4,5,6,7); // Conexión con el LCD

long slpresion = 1020; // Variable para almacenar la presión actual al nivel del mar

float humedad, temperatura, presion, altitud; // Variables que almacenarán los valores de los sensores

bool error, lastUpdateError;

void setup() {
  // Iniciar la comunicación con los dispositivos
  arduino2.begin(9600); 
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16,2); // Iniciar la LCD y especificar filas y columnas

  Serial.begin(9600); // Iniciar comunicación con el ordenador

  pinMode(LEDCOMMS, OUTPUT);
  pinMode(ERRORPIN, INPUT);

  // Imprimir información sobre el programa
  printLcd(&lcd, "Arduino Core");
  delay(3000);
  printLcd(&lcd, "Altimetro: " + (String) slpresion + " Pa");
  delay(2500);

  error = false;
  lastUpdateError = true;

  printLcd(&lcd, "Estado nominal");
}

void loop() {
  while (digitalRead(ERRORPIN)) {
    printLcd(&lcd, "Error");
    error = true;
    lastUpdateError = true;
  }

  if (!error && lastUpdateError) {
    printLcd(&lcd, "Estado nominal");
    lastUpdateError = false;
  }

  // Leer valores de los sensores
  humedad = sensorDHT.readHumidity();
  temperatura = sensorDHT.readTemperature();
  presion = obtenerPresion();
  altitud = sensorBMP.altitude(presion, slpresion);

  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) { // Si algún valor es nulo, no continuar el programa
      error = true;
  }
  
  // FORMATO DATOS: $TEMPERATURA,HUMEDAD,PRESION,ALTITUD*
  String cadenaDatos = String("$" + (String) temperatura + "," + (String) humedad + "," + (String) presion + "," + (String) altitud + "*"); // Construir la cadena de datos

  printLcd(&lcd, (String) temperatura);
  delay(1000);
  printLcd(&lcd, (String) humedad);
  delay(1000);
  printLcd(&lcd, (String) presion);
  delay(1000);
  printLcd(&lcd, (String) altitud);
  delay(1000);

  sendSerial(&arduino2, cadenaDatos); // Enviar los datos por serial al otro arduino

  printLcd(&lcd, F("Delay"));
  delay(11000);  // 11 s + 4 s = 15 s
  
}

// Rutina para leer la presion del BMP180
double obtenerPresion() {
  char estado;
  double temp, pres;

  estado = sensorBMP.startTemperature();

  if (estado != 0) {
    delay(estado);

    estado = sensorBMP.getTemperature(temp);

    if (estado != 0) {
      estado = sensorBMP.startPressure(3);

      if (estado != 0) {
        delay(estado);

        estado = sensorBMP.getPressure(pres, temp);

        if (estado != 0) {
          return (pres);
          
        }
      }
    }
  }
}

// Rutina para enviar datos por Serial
void sendSerial(SoftwareSerial* sw, String data) {
  size_t bytesWritten = sw->print(data);

  if (bytesWritten == data.length()) {
    digitalWrite(LEDCOMMS, HIGH);
  } else {
    Serial.println("Error sending: " + data);
  }
  delay(100);
  digitalWrite(LEDCOMMS, LOW);
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
