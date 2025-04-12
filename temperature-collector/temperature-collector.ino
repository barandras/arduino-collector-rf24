#include "thingProperties.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Multiplexer control pins
const int S0 = 14; // GPIO14
const int S1 = 12; // GPIO12
const int S2 = 13; // GPIO13
const int S3 = 15; // GPIO15
const int analogPin = A0;

// Voltage divider scale factor (empirically measured)
#define BATTERY_SCALE 5.89

// Sleep duration in microseconds (10s = 10e6)
#define SLEEP_DURATION_US 10e6

// --- Mux channel selector ---
void setMuxChannel(int channel) {
  digitalWrite(S0, bitRead(channel, 0));
  digitalWrite(S1, bitRead(channel, 1));
  digitalWrite(S2, bitRead(channel, 2));
  digitalWrite(S3, bitRead(channel, 3));
}

// --- Convert battery voltage to percentage ---
int voltageToPercentage(float voltage) {
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.0) return 0;
  return (int)(((voltage - 3.0) / (4.2 - 3.0)) * 100);
}

void setup() {
  Serial.begin(9600);
  delay(200); // Let Serial stabilize

  // Setup MUX pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Init temp/humidity sensor
  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT3x sensor!");
    while (1) delay(10);
  }

  // Connect to Arduino IoT Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Wait for connection
  while (ArduinoCloud.connected() == 0) {
    ArduinoCloud.update();
    delay(100);
  }

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // --- Read temperature & humidity ---
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  // --- Moisture sensors ---
  setMuxChannel(0); delay(5);
  moisture_banana = analogRead(analogPin);

  setMuxChannel(1); delay(5);
  moisture_avocado_01 = analogRead(analogPin);

  setMuxChannel(2); delay(5);
  moisture_avocado_02 = analogRead(analogPin);

  setMuxChannel(3); delay(5);
  moisture_avocado_03 = analogRead(analogPin);

  // --- Battery ---
  setMuxChannel(15); delay(5);
  int rawBattery = analogRead(analogPin);
  float sensedVoltage = (rawBattery / 1023.0) * 3.3;
  float actualVoltage = sensedVoltage * BATTERY_SCALE;
  battery = voltageToPercentage(actualVoltage);

  // --- Debug print ---
  Serial.print("Temp: "); Serial.print(temperature); Serial.print("°C, ");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.print("%, ");
  Serial.print("Banana: "); Serial.print(moisture_banana); Serial.print(", ");
  Serial.print("Avocado1: "); Serial.print(moisture_avocado_01); Serial.print(", ");
  Serial.print("Avocado2: "); Serial.print(moisture_avocado_02); Serial.print(", ");
  Serial.print("Avocado3: "); Serial.print(moisture_avocado_03); Serial.print(", ");
  Serial.print("Battery Voltage: "); Serial.print(actualVoltage, 2); Serial.print("V → ");
  Serial.print("Battery: "); Serial.print(battery); Serial.println("%");

  // Give time for cloud sync
  ArduinoCloud.update();
  delay(2000); // Let Arduino Cloud send updates

  // --- Go to deep sleep ---
  Serial.println("Going to sleep for 10 seconds...");
  ESP.deepSleep(SLEEP_DURATION_US);
}

void loop() {
  // Empty — all logic handled in setup()
}
