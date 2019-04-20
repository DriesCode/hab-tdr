/*
  GPS: u-blox NEO 6-M
  SD Card reader: Catalex microSD Card Adapter

  GPS TX: PIN 3 ARD
  GPS RX: PIN 4 ARD
  SD Card CS: PIN 10 ARD
  SD Card MOSI: PIN 11 ARD
  SD Card MISO: PIN 12 ARD
  SD Card SCK: PIN 13 ARD
*/

#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

File dataFile; // Archivo logs donde guardaremos los datos
SoftwareSerial ss(4,3); // Conexión con el módulo GPS
TinyGPSPlus gps; // Objeto GPS que nos permitirá la abstracción

String zero = "0"; // Definir un String "zero" para su posterior uso formateando la hora

double lati, longi; // Variables que guardarán los valores de longitud y latitud

void setup() {
  Serial.begin(9600); // Iniciar seriales en 9600 baud/s
  ss.begin(9600);

  while (!Serial) { ; } // Esperar a que se inicie el serial

  if (!SD.begin()) { Serial.println("Error iniciando la tarjeta SD"); while (1) { ; } } // Si hay un problema iniciando la tarjeta SD, no ejecutar siguiente código
}

void loop() {
  zero = "0"; // En cada ciclo del programa, resetear la cadena "zero"
  while (ss.available() > 0) gps.encode(ss.read()); // Abastecer al objeto gps con datos del sensor GPS

  if (!gps.location.isUpdated()) return; // Si los datos no están actualizados, no ejecutar siguiente código

  lati = (double) gps.location.lat(); // Actual latitud GPS
  longi = (double) gps.location.lng(); // Actual longitud GPS
  
  // Tiempo de la captación de datos
  uint8_t hora = gps.time.hour(); 
  uint8_t minuto = gps.time.minute();
  uint8_t segundo = gps.time.second();

  // Si el número (hora/minuto/segundo) tiene solo un dígito, añadirle un cero a la izquierda
  String horaS = ((hora < 10 && hora != 0) ? zero += hora : String(hora));
  String minutoS = ((minuto < 10 && minuto != 0) ? zero += minuto : String(minuto));
  String segundoS = ((segundo < 10 && segundo != 0) ? zero += segundo : String(segundo));

  dataFile = SD.open("logs.txt", FILE_WRITE); // Abrir el archivo de logs para escribir en él

  if (!dataFile) { Serial.println("Error abriendo el archivo"); while (1) { ; } } // Si hay un problema abriendo el archivo log, no ejecutar siguiente código

  // Partes de la cadena final que se imprimirá en el archivo logs
  String tiempo = String("[" + horaS + ":" + minutoS + ":" + segundoS + "] ");
  String latitud = String("Latitud: ");
  String longitud = String(" || Longitud: ");

  // Imprimir la información al archivo logs
  dataFile.print(tiempo);
  dataFile.print(latitud); dataFile.print(lati, 6); dataFile.print(longitud); dataFile.print(longi, 6); dataFile.println();
  dataFile.close();
  
  delay(10000); // Guardar los datos cada 10 segundos

}
