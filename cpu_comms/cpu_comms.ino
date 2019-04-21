
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Wire.h>

#define RX_PIN 7 // Pin recibir datos
#define TX_PIN 8 // Pin para enviar datos ( No usado )

#define GPS_RX_PIN 3 // Pin recibir datos GPS
#define GPS_TX_PIN 4 // Pin enviar datos GPS ( No usado )

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

void setup() {
  Serial.begin(9600); // Iniciar comunicación serial con el ordenador
  arduino1.begin(9600); // Iniciar comunicación serial con el otro arduino
  ssGPS.begin(9600); // Iniciar comunicación serial con el GPS
  
  if (!SD.begin()) { Serial.println("Error iniciando la tarjeta SD"); while (1) { ; } } // Iniciar el lector de la tarjeta SD. Si no se inicia, no ejecutar siguiente código
}

void loop() {
  datos = ""; // Reiniciar la cadena de datos en cada iteración del programa
  arduino1.listen(); // Escuchar el puerto del arduino

  if (arduino1.isListening()) { // Asegurarnos de que se está escuchando el puerto correcto
    Serial.println("Listenign arduino");
    while (arduino1.available() > 0) { // Ejecutar mientras recibamos datos del arduino
      char c = arduino1.read(); // Guardar el byte que hemos recibido
      if (c != "-1") datos += c; // Si es diferente a '-1', añadirlo a la cadena de texto
    }
    
    delay(1500);
    Serial.println(datos);
  }

  ssGPS.listen(); // Escuchar el puerto del GPS

  if (ssGPS.isListening()) { // Asegurarnos de que se está escuchando el puerto correcto
    Serial.println("listening gps");
    while (ssGPS.available() > 0) gps.encode(ssGPS.read()); // Preparar los datos del GPS para poder manipularlos con el objeto TinyGPSPlus

    if (!gps.time.isUpdated()) return; // Si los datos de tiempo no están actualizados, no ejecutar siguiente código
    
    getTime(&gps); // Modificar variables que almacenan el valor del tiempo
    
    Serial.println(tiempo);
    
    delay(1500);
  }
}

void getArduinoSensorData(SoftwareSerial* arduino1) {
  if (!arduino1->available() > 0) return;

  Serial.println(arduino1->read());
}

void getLocation(TinyGPSPlus* gps) {
  lati = gps->location.lat(); // Obtener la latitud actual
  longi = gps->location.lng(); // Obtener la longitud actual
}

void getTime(TinyGPSPlus* gps) {
  hora = gps->time.hour(); // Obtener la hora actual
  minuto = gps->time.minute(); // Obtener el minuto actual
  segundo = gps->time.second(); // Obtener el segundo actual

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
