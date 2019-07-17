/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

bool alreadyConnected = false;

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
  WiFiMulti.addAP("MiAdri", "adrian2002grison");

  alreadyConnected = true;
}

String data[4];
String token = "&field";

bool dataToSend = false;
bool gotValues = false;

WiFiClient client;
HTTPClient http;

void loop() {
  String url = "http://api.thingspeak.com/update?api_key=M5Z4I1ZCB8ZD12UI";
  digitalWrite(LED_BUILTIN, LOW);
  if (Serial.available() > 0 && !dataToSend) {
    String d = Serial.readString();
    Serial.println("Got: " + d);
    char d_c[d.length()];

    d.toCharArray(d_c, sizeof(d_c));
    
    char* nextValue;
    nextValue = strtok(d_c, ",");
    int i = 0;
    while (nextValue != NULL) {
      Serial.print("Split " + String(i) + ": ");
      Serial.println(nextValue);
      data[i] = nextValue;
      nextValue = strtok(NULL, ",");
      i++;
    }

    dataToSend = true;
    gotValues = true;
  }

  if (dataToSend && gotValues) {
    Serial.println("Sending data...");
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      Serial.println("Creating URL for GET request...");
      for (int i = 1; i < 5; i++) {
        url.concat(token);
        url.concat(i);
        url.concat(String("="));
        url.concat(data[(i-1)]);
      }

      Serial.println("Final URL: " + url);

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
            String payload = http.getString();
            Serial.println(payload);
            if (payload != "0") {
              dataToSend = false;
              gotValues = false;
              digitalWrite(LED_BUILTIN, HIGH);
              delay(500);
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
