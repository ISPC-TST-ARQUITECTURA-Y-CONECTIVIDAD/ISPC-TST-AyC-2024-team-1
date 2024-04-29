#include <DHT.h>

#define DHTPIN 15     // Pin D15 (GPIO 15) para el sensor DHT22
#define DHTTYPE DHT22 // Tipo de sensor DHT

const int ledPin = 5;        // Pin D5 (GPIO 5) para el dimmer
const int potPin = 35;       // Pin A35 para el potenciómetro
const int buttonPin = 25;    // Pin D25 (GPIO 25) para el botón
const int buttonLedPin = 2;  // Pin D2 (GPIO 2) para el LED del botón

int previousValue = 0;
float smoothedValue = 0;
bool buttonState = false;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Habilita resistencia pull-up interna para el botón
  pinMode(buttonLedPin, OUTPUT);

  dht.begin(); // Inicializar el sensor DHT
}

void loop() {
  // Leer temperatura y humedad del sensor DHT22
  float temperature = dht.readTemperature(); // Leer temperatura en grados Celsius

  // Mostrar la información de temperatura por consola
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Control del LED con el potenciómetro
  int potValue = analogRead(potPin);
  int mappedValue = map(potValue, 0, 4095, 0, 255);
  
  if(mappedValue > previousValue){
    smoothedValue = 0.9 * smoothedValue + 0.1 * mappedValue;
  } else {
    smoothedValue = 0.1 * smoothedValue + 0.9 * mappedValue;
  }
  
  previousValue = mappedValue;
  
  int brightness = int(smoothedValue);
  analogWrite(ledPin, brightness);

  // Leer estado del botón
  bool buttonPressed = digitalRead(buttonPin) == LOW;

  // Comprobar si el estado del botón ha cambiado
  if (buttonPressed && !buttonState) {
    buttonState = true; // Cambiar el estado del botón

    // Alternar estado del LED del botón
    digitalWrite(buttonLedPin, !digitalRead(buttonLedPin));
  } else if (!buttonPressed && buttonState) {
    buttonState = false; // Restablecer el estado del botón
  }

  delay(100); // Ajusta el retardo según sea necesario
}
