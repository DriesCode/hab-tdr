
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Wire.h>

#define RX_PIN 7 // Pin recibir datos
#define TX_PIN 8 // Pin para enviar datos ( No usado )

#define GPS_RX_PIN 3 // Pin recibir datos GPS
#define GPS_TX_PIN 4 // Pin enviar datos GPS ( No usado )

#define LEDCOMMS 6 // LED que brillará al recibir datos
#define ERRORPIN 2 // Pin que se activará si hay un error

File logs; // Archivo donde guardar logs
SoftwareSerial arduino1 (RX_PIN, TX_PIN); // Conexión con el otro arduino
SoftwareSerial ssGPS (GPS_RX_PIN, GPS_TX_PIN); // Conexión con el GPS
TinyGPSPlus gps; // Objeto que nos permitirá la abstracción

String zero = "0"; // String necesario para formatear la hora

double lati, longi; // Variables donde se guardarán la latitud y longitud
float temperatura, presion, humedad, altitud; // Variables donde se guardarán los datos de los sensores
uint8_t hora, minuto, segundo; // Variables donde se guardarán los valores numéricos del tiempo
String horaStr, minutoStr, segundoStr, tiempo, latiStr, longiStr; // Variables donde se guardarán las cadenas de texto que se imprimirán en el archivo logs

String datos; // Cadena de datos que se reciben del otro arduino

bool newData;
bool endData;
bool newGPS;

void setup() {
  Serial.begin(9600); // Iniciar comunicación serial con el ordenador
  arduino1.begin(9600); // Iniciar comunicación serial con el otro arduino
  ssGPS.begin(9600); // Iniciar comunicación serial con el GPS

  pinMode(LEDCOMMS, OUTPUT);
  pinMode(ERRORPIN, OUTPUT);

  newData = false;
  newGPS = false;
  
  if (!SD.begin()) { Serial.println(F("Error iniciando la tarjeta SD")); while (1) { ; } } // Iniciar el lector de la tarjeta SD. Si no se inicia, no ejecutar siguiente código

  while (Serial.available() > 0) Serial.read();
  arduino1.listen();
  while (arduino1.available() > 0) arduino1.read();
  ssGPS.listen();
  while (ssGPS.available() > 0) ssGPS.read();

  digitalWrite(ERRORPIN, LOW);

  arduino1.listen();
}

void loop() {
  getSensorData(&arduino1); // Datos meteo

  // GPS

  ssGPS.listen();

  if (ssGPS.isListening() && newGPS) {
    while (ssGPS.available()) gps.encode(ssGPS.read());

    if (!gps.time.isUpdated()) return;

    getTime(&gps);

    Serial.println(tiempo);
  }

  if (datos.length() == 27) { // Parser datos
    unsigned int tamDatos = (datos.length())+1;
    char c_datos [tamDatos];
  
    datos.toCharArray(c_datos, sizeof(c_datos));
    c_datos[(sizeof(c_datos)-1)] = ',';
    c_datos[sizeof(c_datos)] = '\0';
    
    /*
      j = 1 -> Temperatura
      j = 2 -> Humedad
      j = 3 -> Presion
      j = 4 -> Altitud
    */
    
    for (int i=0, j=0, k=0, ki = 0; i < sizeof(c_datos); i++) {
      if (c_datos[i] == ','){
        ki = k;
        k = i;
        j++;
  
        unsigned int numCarac;
  
        if (j == 1) {
          numCarac = (k-ki); // Numero de caracteres del valor
        }else {
          numCarac = ((k-ki)-1); // Numero de caracteres del valor
        }
  
        if (j == 1) {
          char temp_str[numCarac];
          strncpy (temp_str, &c_datos[ki], numCarac);
          temp_str[sizeof(temp_str)] = '\0';
  
          temperatura = atof(temp_str);
        } else if (j == 2) {
          char hum_str[numCarac];
          strncpy (hum_str, &c_datos[++ki], numCarac);
          hum_str[sizeof(hum_str)] = '\0';
  
          humedad = atof(hum_str);
        } else if (j == 3) {
          char pres_str[numCarac];
          strncpy(pres_str, &c_datos[++ki], numCarac);
          pres_str[sizeof(pres_str)] = '\0';
  
          presion = atof(pres_str);
        } else if (j == 4) {
          char alt_str[numCarac];
          strncpy(alt_str, &c_datos[++ki], numCarac);
          alt_str[sizeof(alt_str)] = '\0';
  
          altitud = atof(alt_str);
        } else {
          Serial.println("Error j");
          digitalWrite(ERRORPIN, HIGH);
        }
      }
    }
  }

  arduino1.listen();
  newGPS = false;
  
  delay(100);
}

void getSensorData(SoftwareSerial* arduino) {
  if (arduino->isListening()) { 
    if (arduino->available() > 0) {
      char nextc = arduino->read();
      if (nextc == '$') {
        newData = true;
        datos = ""; // Reiniciar la cadena de datos en cada iteración del programa
        char ch = ' ';
        while (arduino->available() > 0 && newData && !endData) {
          ch = arduino->read();
          if (ch == 42) {
            endData = true;
          }else {
            datos += ch;  
          }
        }
      }
    }
    
    if (newData && endData) {
      Serial.println("Datos: " + datos);
      digitalWrite(LEDCOMMS, HIGH);
      delay(100);
      digitalWrite(LEDCOMMS, LOW);
      newGPS = true;
    }
    
    newData = false;
    endData = false;
    
  }
}

void getLocation(TinyGPSPlus* gps) {
  lati = gps->location.lat(); // Obtener la latitud actual
  longi = gps->location.lng(); // Obtener la longitud actual
}

void getTime(TinyGPSPlus* gps) {
  hora = gps->time.hour(); // Obtener la hora actual
  minuto = gps->time.minute(); // Obtener el minuto actual
  segundo = gps->time.second(); // Obtener el segundo actual

  hora += 2; // Convertir a hora local

  // Si cualquier valor (minuto/segundo/hora) solo tiene un dígito, añadirle un cero a la izquierda
  
  horaStr = ((hora < 10 && hora != 0) ? zero += hora : String(hora)); 
  zero = "0";
  minutoStr = ((minuto < 10 && minuto != 0) ? zero += minuto : String(minuto));
  zero = "0";
  segundoStr = ((segundo < 10 && segundo != 0) ? zero += segundo : String(segundo));
  zero = "0";

  // Si cualquier valor es cero, añadir otro

  horaStr = ((hora == 0) ? String("00") : String(horaStr));
  minutoStr = ((minuto == 0) ? String("00") : String(minutoStr));
  segundoStr = ((segundo == 0) ? String("00") : String(segundoStr));

  tiempo = String("[" + horaStr + ":" + minutoStr + ":" + segundoStr + "] "); // Agrupar los datos en una cadena de texto conjunta
}
