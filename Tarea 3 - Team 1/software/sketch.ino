#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Configuración de la red WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuración del servidor MQTT
const char* mqtt_server = "138.59.247.221";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_topic_temp = "temperatura";
const char* mqtt_topic_hum = "humedad";
const char* mqtt_topic_led1 = "led1";
const char* mqtt_topic_led2 = "led2";

// Configuración del pin del sensor DHT22
#define DHT_PIN 14
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

const int buttonPin = 25;    // Botón conectado al pin 25
const int ledPin1 = 22;      // LED conectado al pin 22
const int ledPin2 = 23;      // LED conectado al pin 23
const int potPin = 35;       // Potenciómetro conectado al pin 35

WiFiClient espClient;
PubSubClient client(espClient);

float smoothedValue = 0;  // Valor suavizado del potenciómetro
int previousValue = 0;    // Valor previo del potenciómetro
bool led1State = false;   // Estado actual del LED1

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a la red WiFi");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, estado ");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(buttonPin, INPUT_PULLUP); // Configura el pin del botón como entrada con pull-up
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer la temperatura o humedad desde el sensor DHT22");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C\t");
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("LED1: ");
  Serial.print(led1State ? "ON" : "OFF");
  Serial.print("\t");
  Serial.print("LED2 Brightness: ");
  Serial.println(smoothedValue);

  // Envío de temperatura y humedad como mensajes independientes
  char tempBuffer[10];
  dtostrf(temperatura, 4, 2, tempBuffer);
  client.publish(mqtt_topic_temp, tempBuffer);

  char humBuffer[10];
  dtostrf(humedad, 4, 2, humBuffer);
  client.publish(mqtt_topic_hum, humBuffer);

  // Lectura del estado del botón y control del LED1
  int buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    led1State = !led1State; // Cambia el estado del LED1
    digitalWrite(ledPin1, led1State ? HIGH : LOW); // Enciende o apaga el LED1 según el estado actual
    client.publish(mqtt_topic_led1, led1State ? "ON" : "OFF");
    delay(200); // Retardo para evitar rebotes
  }

  // Lectura del valor del potenciómetro y control suavizado de la luminosidad del LED2
  int potValue = analogRead(potPin);
  int mappedValue = map(potValue, 0, 4095, 0, 255);
  
  if(mappedValue > previousValue){
    smoothedValue = 0.9 * smoothedValue + 0.1 * mappedValue;
  } else {
    smoothedValue = 0.1 * smoothedValue + 0.9 * mappedValue;
  }
  
  previousValue = mappedValue;
  
  int brightness = int(smoothedValue);
  analogWrite(ledPin2, brightness); // Controla la luminosidad del LED2
  char brightnessBuffer[4];
  itoa(brightness, brightnessBuffer, 10);
  client.publish(mqtt_topic_led2, brightnessBuffer);

  delay(100); // Pequeño retardo para estabilidad
}
