
#include <SoftwareSerial.h>
#include <SD.h>
#include <TinyGPS++.h>

#define RX_PIN 7 // Pin recibir datos
#define TX_PIN 8 // Pin para enviar datos ( No usado )

#define GPS_RX_PIN 3 // Pin recibir datos GPS
#define GPS_TX_PIN 4 // Pin enviar datos GPS ( No usado )

#define LEDCOMMS 6 // LED que brillará al recibir datos
#define ERRORPIN 2 // Pin que se activará si hay un error
#define LEDSDERROR 9 // LED que brillará al haber error de SD
#define LEDSD 5 // LED que brillará cuando se escriba en la SD

#define SDcspin 10

File logs; // Archivo donde guardar logs ("LOGS.TXT")
SoftwareSerial arduino1 (RX_PIN, TX_PIN); // Conexión con el otro arduino
SoftwareSerial ssGPS (GPS_RX_PIN, GPS_TX_PIN); // Conexión con el GPS
TinyGPSPlus gps; // Objeto que nos permitirá la abstracción

String zero = "0"; // String necesario para formatear la hora

double lati, longi; // Variables donde se guardarán la latitud y longitud
float temperatura, presion, humedad, altitud; // Variables donde se guardarán los datos de los sensores
uint8_t hora, minuto, segundo; // Variables donde se guardarán los valores numéricos del tiempo
String horaStr, minutoStr, segundoStr, tiempo; // Variables donde se guardarán las cadenas de texto que se imprimirán en el archivo logs

String datos; // Cadena de datos que se reciben del otro arduino

bool newData, endData, newGPS, saveData;

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial con el ordenador
  arduino1.begin(9600); // Iniciar comunicación serial con el otro arduino
  ssGPS.begin(9600); // Iniciar comunicación serial con el GPS

  pinMode(LEDCOMMS, OUTPUT);
  pinMode(ERRORPIN, OUTPUT);
  pinMode(LEDSD, OUTPUT);
  pinMode(LEDSDERROR, OUTPUT);

  newData = newGPS = saveData = false;

  digitalWrite(LEDSD, HIGH); // Check LED
  delay(25);
  digitalWrite(LEDSD, LOW);

  digitalWrite(LEDSDERROR, HIGH); // Check LED
  delay(25);
  digitalWrite(LEDSDERROR, LOW);

  digitalWrite(LEDCOMMS, HIGH); // Check LED
  delay(25);
  digitalWrite(LEDCOMMS, LOW);
  
  
  if (!SD.begin(SDcspin)) { digitalWrite(LEDSDERROR, HIGH); while (1); } // Iniciar el lector de la tarjeta SD. Si no se inicia, no ejecutar siguiente código

  // Liberar buffers

  ssGPS.listen();
  while (ssGPS.available() > 0) ssGPS.read();
  arduino1.listen();
  while (arduino1.available() > 0) arduino1.read();
}

void loop() {
  getSensorData(&arduino1); // Datos meteo
  // GPS

  ssGPS.listen();

  if (ssGPS.isListening() && newGPS) {
    while (ssGPS.available()) gps.encode(ssGPS.read());

    if (!gps.time.isUpdated()) return;

    getTime(&gps);

    if (!gps.location.isUpdated()) return;

    getLocation(&gps);
  }

  // Algoritmo parsear datos
  
  if (datos.length() > 0) {
    unsigned int tamDatos = datos.length()+1;
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
    
    for (int i=0, numComa=0, lecturaActual=0, lecturaAnterior = 0; i < sizeof(c_datos); i++) {
      if (c_datos[i] == ','){
        lecturaAnterior = lecturaActual;
        lecturaActual = i;
        numComa++;
  
        unsigned int numCarac;
  
        if (numComa == 1) {
          numCarac = (lecturaActual-lecturaAnterior); // Numero de caracteres del valor
        }else {
          numCarac = ((lecturaActual-lecturaAnterior)-1); // Numero de caracteres del valor
        }
  
        if (numComa == 1) {
          char temp_str[numCarac];
          strncpy (temp_str, &c_datos[lecturaAnterior], numCarac);
          temp_str[sizeof(temp_str)] = '\0';
  
          temperatura = atof(temp_str);
        } else if (numComa == 2) {
          char hum_str[numCarac];
          strncpy (hum_str, &c_datos[++lecturaAnterior], numCarac);
          hum_str[sizeof(hum_str)] = '\0';
  
          humedad = atof(hum_str);
        } else if (numComa == 3) {
          char pres_str[numCarac];
          strncpy(pres_str, &c_datos[++lecturaAnterior], numCarac);
          pres_str[sizeof(pres_str)] = '\0';
  
          presion = atof(pres_str);
        } else if (numComa == 4) {
          char alt_str[numCarac];
          strncpy(alt_str, &c_datos[++lecturaAnterior], numCarac);
          alt_str[sizeof(alt_str)] = '\0';
  
          altitud = atof(alt_str);
        } else {
          digitalWrite(ERRORPIN, HIGH); // Si hay error, activar pin
        }
      }
    }
  }
  // End parser

  // Save data to SD
  if (saveData) {
    digitalWrite(LEDSD, HIGH);
    logs = SD.open("LOGS.TXT", FILE_WRITE);

    if (!logs) { Serial.println(F("Error abriendo archivo LOGS.TXT")); digitalWrite(LEDSDERROR, HIGH); while(1); }
  
    logs.print(tiempo); // HORA: [hh:mm:ss]
    logs.print(F("(")); logs.print(lati, 6); logs.print(F(" | ")); logs.print(longi, 6); logs.print(F(") ")); // POSICIÓN: (XX.XXXX ; XX.XXXX)
    logs.print("Temperatura: " + (String) temperatura + " ºC;   "); // TEMPERATURA: XX.XX ºC
    logs.print("Humedad: " + (String) humedad + " %;   "); // HUMEDAD: XX.XX %
    logs.print("Presion: " + (String) presion + " Pa;   "); // PRESIÓN: XXXXXX.XX Pa
    logs.print("Altitud: " + (String) altitud + " m"); // ALTITUD: XX.XX m
    logs.println();
  
    // [hh:mm:ss] (LATITUD ; LONGITUD) Altitud: ALTITUD m   ; Humedad: HUMEDAD %   ; Presion: PRESION Pa   ; Altitud: ALTITUD m

    logs.close(); // Cerrar archivo para guardar
    digitalWrite(LEDSD, LOW);
    saveData = false;

    String dataToSerial = String((String)temperatura+","+(String)humedad+","+(String)presion+","+(String)altitud+","+(String)lati+","+(String)longi);
    char dTS_c[dataToSerial.length()];
    dataToSerial.toCharArray(dTS_c, sizeof(dTS_c));
    Serial.write(dTS_c);
  }

  arduino1.listen();
  newGPS = false;
  datos = "";
  temperatura = humedad = presion = altitud = 0;
  
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
      digitalWrite(LEDCOMMS, HIGH);
      delay(100);
      digitalWrite(LEDCOMMS, LOW);
      newGPS = true;
      saveData = true;
    }
    
    newData = false;
    endData = false;
  }
}

// GPS Functions

void getLocation(TinyGPSPlus* gps) {
  //Serial.println(F("pos"));
  lati = gps->location.lat(); // Obtener la latitud actual
  longi = gps->location.lng(); // Obtener la longitud actual
}

void getTime(TinyGPSPlus* gps) {
 // Serial.println(F("tiempo"));
  hora = gps->time.hour(); // Obtener la hora actual
  minuto = gps->time.minute(); // Obtener el minuto actual
  segundo = gps->time.second(); // Obtener el segundo actual

  hora += 2; // Convertir a hora local

  // Si cualquier valor (minuto/segundo/hora) solo tiene un dígito, añadirle un cero a la izquierda
  
  horaStr = (hora < 10) ? zero += hora : String(hora); 
  zero = "0";
  minutoStr = (minuto < 10) ? zero += minuto : String(minuto);
  zero = "0";
  segundoStr = (segundo < 10) ? zero += segundo : String(segundo);
  zero = "0";

  tiempo = String("[" + horaStr + ":" + minutoStr + ":" + segundoStr + "] "); // Agrupar los datos en una cadena de texto conjunta
}
