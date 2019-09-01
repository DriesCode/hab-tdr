#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

bool alreadyConnected = false; // Marcar si estamos ya conectados o no

String data[6]; // Crear un array de String de 6 espacios que contendrá cada dato recibido
String token = "&field"; // Token que se añadirá por cada variable a la petición GET

bool dataToSend = false; // Variable que marcará si estamos listos para enviar datos
bool gotValues = false; // Variable que marcará si hemos recibido datos

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  
  WiFiMulti.addAP("red", "contraseña"); // Conectarse a la red wifi (en este caso, portatil del móvil)
  alreadyConnected = true; // Marcar que ya nos hemos conectado a la red
}

WiFiClient client;
HTTPClient http;

void loop() {
  String url = "http://api.thingspeak.com/update?api_key=M5Z4I1ZCB8ZD12UI"; // URL base que se le añadirá cada token con variable
  if (Serial.available() > 0 && !dataToSend) { // Si tenemos datos en Serial y no tenemos datos para enviar por WIFI, continuar
    String d = Serial.readString(); // Almacenar en variable la cadena de texto que recibimos
    Serial.println("Got: " + d); // Imprimir por pantalla que hemos recibido datos (debugging)
    char d_c[d.length()]; // Crear la varibale d_c con un datos en forma de array de caracteres

    d.toCharArray(d_c, sizeof(d_c)); // Creación propiamente del array d_c
    
    char* nextValue; // Variable que guardará el caracter siguiente para la conversión
    nextValue = strtok(d_c, ","); // Asignarle el valor al primer caracter al romper la cadena por comas
    int i = 0; // Posición del valor
    while (nextValue != NULL) { // Repetir mientras hasta que no haya más caracteres
      Serial.print("Split " + String(i) + ": "); // Mostrar información del caracter actual (debugging)
      Serial.println(nextValue);
      data[i] = nextValue; // Asignar el caracter al array data en la misma posición que tenía en d_c
      nextValue = strtok(NULL, ","); // Reasignar nextValue al siguiente valor
      i++; // Incrementar por uno "i"
    }

    dataToSend = true; // Marcar que ya hemos recibido datos y tenemos datos por enviar
    gotValues = true;
  }

  if (dataToSend && gotValues) { // Continuar si se cumplen las dos condiciones anteriores
    Serial.println("Sending data..."); // Debugging
    if ((WiFiMulti.run() == WL_CONNECTED)) { // Comprobar si estamos conectados al WIFI
      Serial.println("Creating URL for GET request..."); // Debugging
      // Modificar la URL base para que contenga todos los valores de cada variable
      for (int i = 1; i < 7; i++) {
        url.concat(token);
        url.concat(i);
        url.concat(String("="));
        url.concat(data[(i-1)]);
      }

      Serial.println("Final URL: " + url); // Debugging

      Serial.print("[HTTP] begin...\n");
      if (http.begin(client, url)) {  // HTTP
  
  
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            
            String payload = http.getString(); // Recibir la respuesta del servidor
            Serial.println(payload); // Debugging
            if (payload != "0") { // Si la respuesta del servidor es diferente a 0, continuar
              dataToSend = false; // Restablecer variables de estado
              gotValues = false;
              delay(500); // Esperar 500 ms para no sobrecargar
            }
            
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
  
        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    } else {
      dataToSend = true;
    }
  }
}
