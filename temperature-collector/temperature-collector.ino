#include "thingProperties.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

const int analogPin = A0;
#define BATTERY_SCALE 4.03
#define ADC_REF_VOLTAGE 3.3
#define ADC_SAMPLING_DURATION 2000
#define ADC_SAMPLING_INTERVAL 50
#define LOOP_INTERVAL 10000

int voltageToPercentage(float voltage) {
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.0) return 0;
  return (int)(((voltage - 3.0) / (4.2 - 3.0)) * 100);
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT3x sensor! Check wiring.");
    while (1) delay(10);
  }

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("Setup complete.");
}

void loop() {
  unsigned long loopStart = millis();
  ArduinoCloud.update();

  // Temperature and humidity
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  // Sample battery voltage for 2 seconds
  unsigned long start = millis();
  long sum = 0;
  int count = 0;
  while (millis() - start < ADC_SAMPLING_DURATION) {
    int raw = analogRead(analogPin);
    sum += raw;
    count++;
    delay(ADC_SAMPLING_INTERVAL);
  }

  float avgRaw = (float)sum / count;
  float sensedVoltage = (avgRaw / 1023.0) * ADC_REF_VOLTAGE;
  float actualVoltage = sensedVoltage * BATTERY_SCALE;
  battery = voltageToPercentage(actualVoltage);

  // Debug output
  Serial.println("------------------------------------------------");
  Serial.print("Temp: "); Serial.print(temperature); Serial.print(" °C, ");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %, ");
  Serial.print("Avg ADC: "); Serial.print(avgRaw); Serial.print(", ");
  Serial.print("Vout: "); Serial.print(sensedVoltage, 3); Serial.print(" V, ");
  Serial.print("Battery Voltage: "); Serial.print(actualVoltage, 2); Serial.print(" V → ");
  Serial.print("Battery %: "); Serial.print(battery); Serial.println(" %");
  Serial.println("------------------------------------------------\n");

  // Wait to complete 10s total loop
  unsigned long elapsed = millis() - loopStart;
  if (elapsed < LOOP_INTERVAL) {
    delay(LOOP_INTERVAL - elapsed);
  }
}
