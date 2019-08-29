#include <SoftwareSerial.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define RX_PIN 11 // Pin recibir datos en Arduino ( No usado )
#define TX_PIN 10 // Pin enviar datos en Arduino

#define LEDCOMMS 3 // Pin del LED Blanco envío de datos
#define ERRORPIN 12 // Pin activación estado de error

SoftwareSerial arduino2 (RX_PIN, TX_PIN); // Conexión con el arduino Comunicaciones
DHT sensorDHT (2, DHT22); // Conexión con el sensor de temperatura y humedad (AM2302)
SFE_BMP180 sensorBMP; // Conexión con el sensor de presión y altura (BMP180)
LiquidCrystal lcd(8,9,4,5,6,7); // Conexión con el LCD 1602

double presionBase; // Variable para almacenar la referencia de presión base

float humedad, temperatura, presion, altura; // Variables que almacenarán los valores de los sensores

bool error, debug; // Variables para almacenar, respectivamente: Si el estado de error está activado y si está activado el modo debug

double obtenerPresion(); // Declaración de la función obtenerPresion()

void setup() {
  // Iniciar la comunicación con los dispositivos
  arduino2.begin(9600);
  sensorDHT.begin();
  sensorBMP.begin();
  lcd.begin(16,2); // Iniciar la LCD y especificar filas y columnas (16 columnas, 2 filas)

  pinMode(13, INPUT); // Declarar el pin 13 como pin del botón del modo debug, e iniciarlo como entrada
  debug = digitalRead(13); // Leer el valor del pin 13 (debug)

  pinMode(LEDCOMMS, OUTPUT); // Declarar el pin del LED como salida
  pinMode(ERRORPIN, INPUT); // Declarar el pin de error como entrada

  if (debug) { // Si el debug está activado (Pin 13 == HIGH) = (Pin 13 == 1), registrar la referencia de presión base
    printLcd(&lcd, "Debug enabled"); // Indicar por el LCD que el modo debug está activado
    delay(1000); // Esperar 1 segundo (mostrar el texto durante 1 segundo)
    presionBase = obtenerPresion(); // Obtener la presión por la función obtenerPresion() y guardarla en la variable presionBase
    saveEEPROM(0, presionBase); // Guardar la referencia de presión base en la memoria EEPROM del Arduino Sensores
  } else { // Si el modo debug no está activado, en vez de registrar la presión base, leerla de la memoria EEPROM
    readEEPROM(0, presionBase); // Leer de la memoria EEPROM el valor de la presión base y guardarlo en presionBase
  }

  printLcd(&lcd, "Arduino Core"); // Mostrar nombre del programa. En este caso, Arduino Core
  delay(3000); // Mantener el texto durante 3 segundos
  printLcd(&lcd, "Altimetro: " + (String) presionBase + " hPa"); // Mostrar la presión base registrada
  delay(2500); // Mantener el texto durante 2,5 segundos
}

void loop() {
  humedad = sensorDHT.readHumidity(); // Registrar el valor de humedad mediante la librería
  temperatura = sensorDHT.readTemperature(); // Registrar el valor de temperatura mediante la librería
  presion = obtenerPresion(); // Registrar el valor de presión mediante la librería
  altura = sensorBMP.alturae(presion, presionBase); // Registrar el valor de altura mediante la librería

  if (isnan(humedad) || isnan(temperatura) || isnan(presion) || isnan(altura)) { // Si algún valor es nulo, activar estado de error
      error = true;
  } else {
    error = false;
  }

  if (error) { // Si el estado de error está activado, no mostrar "Error" hasta que se haya desactivado este estado
    printLcd (&lcd, "Error");
  } else { // Si no está en estado de error, proseguir con la ejecución del programa
    // Crear la cadena de datos para enviarla al Arduino Comunicaciones mediante Serial. Formato de la cadena: $TEMPERATURA,HUMEDAD,PRESION,altura*
    String cadenaDatos = String("$" + (String) temperatura + "," + (String) humedad + "," + (String) presion + "," + (String) altura + "*"); // Construir la cadena de datos
  
    printLcd(&lcd, F("Temperatura"), (String) temperatura); // Mostrar la temperatura durante 1 segundo
    delay(1000);
    printLcd(&lcd, F("Humedad"), (String) humedad); // Mostrar la humedad durante 1 segundo
    delay(1000);
    printLcd(&lcd, F("Presion"), (String) presion); // Mostrar la presion durante 1 segundo
    delay(1000);
    printLcd(&lcd, F("Altura"), (String) altura); // Mostrar la altura durante 1 segundo
    delay(1000);
  
    sendSerial(&arduino2, cadenaDatos); // Enviar los datos por serial al Arduino comunicaciones
  
    for (int i = 0; i < 116; i++) { // Mostrar un texto de "Espere" y los segundos restantes hasta el próximo registro de datos
      printLcd(&lcd, F("Espere"), (String) (116-i));
      delay(1000);
    }
  }
}


/*

    Rutinas (funciones) de creación propia para simplificar el código de este programa
  
*/


template <class T> void saveEEPROM(int item, T &value) { // Rutina para guardar un valor en la memoria EEPROM de Arduino
  EEPROM.put(item, value);
}

template <class T> void readEEPROM(int item, T &var) { // Rutina para leer un valor de la memoria EEPROM de Arduino
  EEPROM.get(item, var);
}

double obtenerPresion() { // Rutina para leer la presión del sensor BMP180 y retornar el valor
  char estado; // Variable que registrará el estado de cada operación anterior
  double temp, pres; // Variables que almacenarán el valor de presión y temperatura del BMP180

  estado = sensorBMP.startTemperature(); // Comenzar la grabación de temperatura

  if (estado != 0) { // Si no hay ningún error en la medida anterior, continuar programa
    delay(estado); // Esperar el número de segundos necesario hasta poder realizar la grabación de temperatura

    estado = sensorBMP.getTemperature(temp); // Almacenar la temperatura en temp y guardar el estado de la operación en estado

    if (estado != 0) { // Si no hay ningun error en la medida anterior, continuar programa
      estado = sensorBMP.startPressure(3); // Comenzar la grabación de la temperatura con máxima precisión (3)

      if (estado != 0) { // Si no hay ningun error en la medida anterior, continuar programa
        delay(estado); // Esperar el número de segundos necesario hasta poder realizar la grabación de presion

        estado = sensorBMP.getPressure(pres, temp); // Guardar la presión en pres, utilizando como referencia la temperatura de temp, y almacenar el estado en estado

        if (estado != 0) { // Si no hay ningun error en la medida anterior, continuar programa
          return (pres); // Retornar el valor de presión
        } else error = true;
      } else error = true;
    } else error = true;
  } else error = true;
}


void sendSerial(SoftwareSerial* sw, String data) { // Rutina para enviar datos por Serial (En este caso, al Arduino Comunicaciones)
  size_t bytesWritten = sw->print(data); // Enviar los datos de data por serial utilizando los pines de sw y almacenar los bytes enviados en bytesWritten

  if (bytesWritten == data.length()) { // Si se ha enviado la misma cantidad de datos que la que se quería enviar, indicar que la operación fue un éxito
    digitalWrite(LEDCOMMS, HIGH); // Encender el LED blanco durante 100 ms
  } else {
    printLcd(&lcd, "Error sending data"); // Si no fue un éxito (no enviamos la misma cantidad de datos que queríamos enviar), entrar en estado de error
    error = true;
  }
  delay(100);
  digitalWrite(LEDCOMMS, LOW); // Apagar el LED blanco
}


void printLcd(LiquidCrystal* lcd, String text) { // Rutina para imprimir texto en la LCD con formato automático
  lcd->clear(); // Borrar el texto del LCD anterior antes de empezar a escribir
  lcd->setCursor(0,0); // Poner el cursor en la posición inicial
  if (text.length() > 16) { // Si el número de caracteres es mayor a 16, se utilizarán ambas filas
    for (int i = 0; i < text.length(); i++) { // Imprimir los primeros 16 caracteres en la primera fila y los restantes en la fila de abajo
      if (i == 16) { // Si se llega a la última columna, posicionar el cursor en la fila segunda
        lcd->setCursor(0, 1);
      }
      lcd->print(text.charAt(i));
    }
  } else { // Si el texto no supera los 16 caracteres, imprimir en la primera fila
    lcd->print(text);
  }
}

void printLcd(LiquidCrystal* lcd, String text1, String text2) { // Rutina para imprimir texto en la LCD indicando qué texto irá en la fila primera y cual en la segunda
  lcd->clear(); // Borrar texto del LCD
  lcd->setCursor(0,0); // Posicionar cursor al inicio de la primera fila
  lcd->print(text1); // Mostrar primer texto
  lcd->setCursor(0,1); // Posicionar cursor en el inicio de la segunda fila
  lcd->print(text2); // Mostrar segundo texto
}
