
#include <SoftwareSerial.h>
#include <SD.h>
#include <TinyGPS++.h>

#define RX_PIN 7 // Pin recibir datos de Arduino Sensores
#define TX_PIN 8 // Pin para enviar datos ( No usado )

#define GPS_RX_PIN 3 // Pin recibir datos GPS
#define GPS_TX_PIN 4 // Pin enviar datos GPS ( No usado )

#define LEDCOMMS 6 // LED que brillará al recibir datos
#define ERRORPIN 2 // Pin que se activará si hay un error
#define LEDSDERROR 9 // LED que brillará al haber error de SD
#define LEDSD 5 // LED que brillará cuando se escriba en la SD

#define SDcspin 10 // Pin CS del microSD Card logger

File logs; // Archivo donde guardar logs ("LOGS.TXT")
SoftwareSerial arduino1 (RX_PIN, TX_PIN); // Conexión con el otro arduino
SoftwareSerial ssGPS (GPS_RX_PIN, GPS_TX_PIN); // Conexión con el GPS
TinyGPSPlus gps; // Objeto que nos permitirá la abstracción

String zero = "0"; // String necesario para formatear la hora

double lati, longi; // Variables donde se guardarán la latitud y longitud
float temperatura, presion, humedad, altura; // Variables donde se guardarán los datos de los sensores
uint8_t hora, minuto, segundo; // Variables donde se guardarán los valores numéricos del tiempo
String horaStr, minutoStr, segundoStr, tiempo; // Variables donde se guardarán las cadenas de texto que se imprimirán en el archivo logs

String datos; // Cadena de datos que se reciben del otro arduino

bool newData, endData, newGPS, saveData; // Variables que indicarán el estado del programa

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial con el ESP8266
  arduino1.begin(9600); // Iniciar comunicación serial con el otro arduino
  ssGPS.begin(9600); // Iniciar comunicación serial con el GPS

  // Iniciar los pines de LED como salidas
  pinMode(LEDCOMMS, OUTPUT);
  pinMode(ERRORPIN, OUTPUT);
  pinMode(LEDSD, OUTPUT);
  pinMode(LEDSDERROR, OUTPUT);

  newData = newGPS = saveData = false; // Iniciar variables de estado del programa en false

  digitalWrite(LEDSD, HIGH); // Comprobar que LED funcione
  delay(25);
  digitalWrite(LEDSD, LOW);

  digitalWrite(LEDSDERROR, HIGH); // Comprobar que LED funcione
  delay(25);
  digitalWrite(LEDSDERROR, LOW);

  digitalWrite(LEDCOMMS, HIGH); // Comprobar que LED funcione
  delay(25);
  digitalWrite(LEDCOMMS, LOW);
  
  
  if (!SD.begin(SDcspin)) { digitalWrite(LEDSDERROR, HIGH); while (1); } // Iniciar el lector de la tarjeta SD. Si no se inicia, no ejecutar siguiente código

  // Liberar buffers, por si hay datos basura en los Seriales

  ssGPS.listen();
  while (ssGPS.available() > 0) ssGPS.read();
  arduino1.listen();
  while (arduino1.available() > 0) arduino1.read();
}

void loop() {
  getSensorData(&arduino1); // Obtener los datos enviados por Arduino sensores

  ssGPS.listen(); // Comenzar a escuchar al puerto del GPS

  if (ssGPS.isListening() && newGPS) { // Leer GPS si se está escuchando su puerto y la variable newGPS es true
    while (ssGPS.available()) gps.encode(ssGPS.read()); // Decodificar datos obtenidos por GPS del buffer

    if (!gps.time.isUpdated()) return; // Si no ha cambiado la hora, no continuar
    getTime(&gps); // Leer hora
    
    if (!gps.location.isUpdated()) return; // Si no ha cambiado la posición, no continuar
    getLocation(&gps); // Leer posición
  }
  
  if (datos.length() > 0) { // Inicio algoritmo para decodificar datos de Arduino sensores
    unsigned int tamDatos = datos.length()+1; // Guardar el tamaño de datos en variable
    char c_datos [tamDatos]; // Crear un array de caracteres con el tamaño de la cadena de datos
  
    datos.toCharArray(c_datos, sizeof(c_datos)); // Generar un array de caracteres de la cadena de datos recibida
    c_datos[(sizeof(c_datos)-1)] = ','; // Posicionar una coma como penúltimo elemento del array
    c_datos[sizeof(c_datos)] = '\0'; // Posicionar un \0 al final del array indicando que finaliza la cadena de caracteres (String)
    
    /*
      numComa = 1 -> Temperatura
      numComa = 2 -> Humedad
      numComa = 3 -> Presion
      numComa = 4 -> altura
    */

    for (int i=0, numComa=0, lecturaActual=0, lecturaAnterior = 0; i < sizeof(c_datos); i++) { // Bucle for que itera por cada caracter de la cadena de datos
      // Si el valor iterado actual es una coma, actualizar variables de estado del bucle
      if (c_datos[i] == ','){
        lecturaAnterior = lecturaActual; // Actualizar anterior posicion de lectura de caracter
        lecturaActual = i; // Actualizar posicion de lectura de caracter
        numComa++; // Sumar 1 al numero de comas leido
  
        unsigned int numCarac; // Variable almacenará el numero de caracteres a leer
  
        if (numComa == 1) {
          numCarac = (lecturaActual-lecturaAnterior); // Numero de caracteres del valor si la coma es la primera (no anterior coma)
        }else {
          numCarac = ((lecturaActual-lecturaAnterior)-1); // Numero de caracteres del valor si la coma no es la primera (contando coma anterior)
        }
  
        if (numComa == 1) { // Si es la primera coma, leer temperatura
          char temp_str[numCarac]; // Crear array para los caracteres del valor
          strncpy (temp_str, &c_datos[lecturaAnterior], numCarac); // Copiar los caracteres del valor del array de la cadena de datos al array de la temperatura
          temp_str[sizeof(temp_str)] = '\0'; // Añadir signo \0 indicando final de cadena de caracteres (String)
  
          temperatura = atof(temp_str); // Convertir caracteres a valor numero de coma flotante (float)
        } else if (numComa == 2) { // Si es la segunda, humedad
          char hum_str[numCarac];
          strncpy (hum_str, &c_datos[++lecturaAnterior], numCarac);
          hum_str[sizeof(hum_str)] = '\0';
  
          humedad = atof(hum_str);
        } else if (numComa == 3) { // Tercera, presion
          char pres_str[numCarac];
          strncpy(pres_str, &c_datos[++lecturaAnterior], numCarac);
          pres_str[sizeof(pres_str)] = '\0';
  
          presion = atof(pres_str);
        } else if (numComa == 4) { // Cuarta, altura
          char alt_str[numCarac];
          strncpy(alt_str, &c_datos[++lecturaAnterior], numCarac);
          alt_str[sizeof(alt_str)] = '\0';
  
          altura = atof(alt_str);
        } else {
          digitalWrite(ERRORPIN, HIGH); // Si hay error, activar pin para activar estado de error de Arduino sensores
        }
      }
    }
  }

  if (saveData) { // Si saveData es true, guardar nuevos datos en LOGS.txt
    digitalWrite(LEDSD, HIGH); // Encender LED azul al empezar el guardado
    logs = SD.open("LOGS.TXT", FILE_WRITE); // Abrir archivo LOGS.txt

    if (!logs) { Serial.println(F("Error abriendo archivo LOGS.TXT")); digitalWrite(LEDSDERROR, HIGH); while(1); } // Si no se ha abierto correctamente, no continuar y encender LED rojo

    // Escribir el texto con los valores en el archivo LOGS.txt
    logs.print(tiempo); // HORA: [hh:mm:ss]
    logs.print(F("(")); logs.print(lati, 6); logs.print(F(" | ")); logs.print(longi, 6); logs.print(F(") ")); // POSICIÓN: (XX.XXXX ; XX.XXXX)
    logs.print("Temperatura: " + (String) temperatura + " ºC;   "); // TEMPERATURA: XX.XX ºC
    logs.print("Humedad: " + (String) humedad + " %;   "); // HUMEDAD: XX.XX %
    logs.print("Presion: " + (String) presion + " Pa;   "); // PRESIÓN: XXXXXX.XX Pa
    logs.print("altura: " + (String) altura + " m"); // altura: XX.XX m
    logs.println();
  
    // Formato de datos de LOGS: [hh:mm:ss] (LATITUD ; LONGITUD) altura: altura m   ; Humedad: HUMEDAD %   ; Presion: PRESION Pa   ; altura: altura m

    logs.close(); // Cerrar archivo para guardar
    digitalWrite(LEDSD, LOW); // Apagar LED azul indicando que ha finalizado la operación
    saveData = false; // Establecer saveData a false

    String dataToSerial = String((String)temperatura+","+(String)humedad+","+(String)presion+","+(String)altura+","); // Crear cadena de datos para enviarlos al ESP8266

    // Enviar la cadena de datos al ESP8266, Latitud y Longitud de GPS con 6 decimales de aproximación
    Serial.print(dataToSerial);
    Serial.print(lati, 6);
    Serial.print(",");
    Serial.print(longi, 6);
  }

  arduino1.listen(); // Volver a escuchar al puerto del Arduino para volver a leer cadena de datos
  newGPS = false; // Indicar que no se vuelva a leer los datos del GPS hasta que se lean datos Arduino
  datos = ""; // Restablecer cadena de datos
  temperatura = humedad = presion = altura = 0; // Restablecer variables
  
  delay(100); // Esperar 100 ms por precaución
}

/* 
 *   
 *  Rutinas de creación propia para simplificar código principal
 *  
 */

void getSensorData(SoftwareSerial* arduino) { // Rutina para leer la cadena de datos del Serial
  if (arduino->isListening()) {  // Asegurar que se está escuchando el puerto de Arduino
    if (arduino->available() > 0) { // Continuar si hay datos nuevos en el buffer del Serial
      char nextc = arduino->read(); // Leer primer caracter
      if (nextc == '$') { // Continuar si el caracter es $ 
        newData = true;
        datos = ""; // Reiniciar la cadena de datos en cada iteración del programa
        char ch = ' '; // Variable para almacenar el siguiente caracter leido
        while (arduino->available() > 0 && newData && !endData) { // Continuar si hay datos en buffer, newData es true y endData es false
          ch = arduino->read(); // Leer el caracter
          if (ch == 42) { // Si el caracter es * (ASCII 42), detener lectua
            endData = true;
          }else {
            datos += ch;  // Si no es *, añadir caracter a la cadena de datos
          }
        }
      }
    }
    
    if (newData && endData) { // Si ya se ha acabado de leer, parpadear LED blanco
      digitalWrite(LEDCOMMS, HIGH);
      delay(100);
      digitalWrite(LEDCOMMS, LOW);
      newGPS = true; // newGPs true para leer GPS en el ciclo
      saveData = true; // saveData true para guardar los datos en SD
    }
    
    newData = false; // newData false y endData false porque ya se ha acabado de leer
    endData = false;
  }
}


void getLocation(TinyGPSPlus* gps) { // Rutina para guardar la posicion de GPS en variables
  lati = gps->location.lat(); // Obtener la latitud actual
  longi = gps->location.lng(); // Obtener la longitud actual
}

void getTime(TinyGPSPlus* gps) { // Rutina para guardar el tiempo de GPS en variables
  hora = gps->time.hour(); // Obtener la hora actual
  minuto = gps->time.minute(); // Obtener el minuto actual
  segundo = gps->time.second(); // Obtener el segundo actual

  hora += 2; // Convertir a hora local

  if (hora >= 24) hora-=24;

  // Si cualquier valor (minuto/segundo/hora) solo tiene un dígito, añadirle un cero a la izquierda
  
  horaStr = (hora < 10) ? zero += hora : String(hora); 
  zero = "0";
  minutoStr = (minuto < 10) ? zero += minuto : String(minuto);
  zero = "0";
  segundoStr = (segundo < 10) ? zero += segundo : String(segundo);
  zero = "0";

  tiempo = String("[" + horaStr + ":" + minutoStr + ":" + segundoStr + "] "); // Agrupar los datos en una cadena de texto conjunta
}
