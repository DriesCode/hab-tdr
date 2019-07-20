#include <SoftwareSerial.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define RX_PIN 11 // Pin recibir datos en Arduino ( No usado )
#define TX_PIN 10 // Pin enviar datos en Arduino

#define LEDCOMMS 3
#define ERRORPIN 12

SoftwareSerial arduino2 (RX_PIN, TX_PIN); // Conexión con el arduino
DHT sensorDHT (2, DHT22); // Conexión con el sensor de temperatura y humedad
SFE_BMP180 sensorBMP; // Conexión con el sensor de presión y altitud
LiquidCrystal lcd(8,9,4,5,6,7); // Conexión con el LCD

double presionBase; // Variable para almacenar la presión actual al nivel del mar

float humedad, temperatura, presion, altitud; // Variables que almacenarán los valores de los sensores

bool error, debug;

double obtenerPresion();

void setup() {
  // Iniciar la comunicación con los dispositivos
  arduino2.begin(9600);
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16,2); // Iniciar la LCD y especificar filas y columnas

  Serial.begin(9600); // Iniciar comunicación con el ordenador

  pinMode(13, INPUT);
  debug = digitalRead(13);

  pinMode(LEDCOMMS, OUTPUT);
  pinMode(ERRORPIN, INPUT);

  // Get baseline pressure if debug is enabled
  if (debug) {
    printLcd(&lcd, "Debug enabled");
    delay(1000);
    presionBase = obtenerPresion();
    saveEEPROM(0, presionBase);

    Serial.println(presionBase);
  } else {
    readEEPROM(0, presionBase);
    Serial.println(presionBase);
  }

  // Imprimir información sobre el programa
  printLcd(&lcd, "Arduino Core");
  delay(3000);
  printLcd(&lcd, "Altimetro: " + (String) presionBase + " hPa");
  delay(2500);
}

void loop() {
  // Leer datos de los sensores

  humedad = sensorDHT.readHumidity();
  temperatura = sensorDHT.readTemperature();
  presion = obtenerPresion();
  altitud = sensorBMP.altitude(presion, presionBase);

  // Comprobar si hay error

  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altitud)) { // Si algún valor es nulo, no continuar el programa
      error = true;
  } else {
    error = false;
  }

  /*if (!error) {
    error = digitalRead(ERRORPIN);
  }*/

  // Si hay un error, no ejecutar programa e imprimir "Error"
  if (error) {
    printLcd (&lcd, "Error");
  } else {
    // FORMATO DATOS: $TEMPERATURA,HUMEDAD,PRESION,ALTITUD*
    String cadenaDatos = String("$" + (String) temperatura + "," + (String) humedad + "," + (String) presion + "," + (String) altitud + "*"); // Construir la cadena de datos
  
    printLcd(&lcd, F("Temperatura"), (String) temperatura);
    delay(1000);
    printLcd(&lcd, F("Humedad"), (String) humedad);
    delay(1000);
    printLcd(&lcd, F("Presion"), (String) presion);
    delay(1000);
    printLcd(&lcd, F("Altura"), (String) altitud);
    delay(1000);
  
    sendSerial(&arduino2, cadenaDatos); // Enviar los datos por serial al otro arduino
  
    for (int i = 0; i < 116; i++) {
      printLcd(&lcd, F("Espere"), (String) (116-i));
      delay(1000);
    }
  }
}

// Rutina para guardar datos en EEPROM
template <class T> void saveEEPROM(int item, T &value) {
  EEPROM.put(item, value);
}

// Rutina para leer datos en EEPROM
template <class T> void readEEPROM(int item, T &var) {
  EEPROM.get(item, var);
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
        } else error = true;
      } else error = true;
    } else error = true;
  } else error = true;
}

// Rutina para enviar datos por Serial
void sendSerial(SoftwareSerial* sw, String data) {
  size_t bytesWritten = sw->print(data);

  if (bytesWritten == data.length()) {
    digitalWrite(LEDCOMMS, HIGH);
  } else {
    printLcd(&lcd, "Error sending data");
    error = true;
  }
  delay(100);
  digitalWrite(LEDCOMMS, LOW);
}

// Rutina para imprimir texto en la LCD
void printLcd(LiquidCrystal* lcd, String text) {
  lcd->clear();
  lcd->setCursor(0,0);
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

// Segunda rutina. text1.length() < 16 // text2.length() < 16
void printLcd(LiquidCrystal* lcd, String text1, String text2) {
  lcd->clear();
  lcd->setCursor(0,0);
  lcd->print(text1);
  lcd->setCursor(0,1);
  lcd->print(text2);  
}
