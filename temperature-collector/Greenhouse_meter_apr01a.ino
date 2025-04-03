#include "thingProperties.h"
#include <Wire.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(1500); // wait for Serial Monitor

  // Initialize sensor
  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT3x sensor! Check wiring.");
    while (1) delay(10);
  }

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Debug output level
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("Setup complete.");
}

void loop() {
  ArduinoCloud.update();

  // Read temperature and humidity from SHT3x
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (!isnan(t)) {
    temperature = t;
  }

  if (!isnan(h)) {
    humidity = h;
  }

  // Read raw soil moisture from A0
  moisture_banana = analogRead(A0); // raw analog value, 0–1023

  // Log to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Raw Moisture: ");
  Serial.println(moisture_banana);

  delay(10000); // 10s delay to match cloud's periodic update
}
