#include "RF24.h"
#include "printf.h"
#include <Wire.h>
#include "SHTSensor.h"

// Hardware configuration
RF24 radio(9, 10);  // Set up nRF24L01 radio on SPI bus plus pins 9 & 10

// Radio pipe addresses for the master node to communicate.
const uint64_t addresses[] = {0xF0F0F0F0E1LL};

SHTSensor sht;

struct Payload {
  float temperature;
  float humidity;
};

void setup(void)
{

  Serial.begin(115200);
  printf_begin();                 // needed for printDetails()

  // Print preamble
  Serial.println(F("Start sending measure data"));

  // Setup and configure rf radio
  radio.begin();
  radio.enableDynamicPayloads();  // Enable dynamic payloads
  radio.setRetries(5, 15);        // delay between retries = 5 * 250 + 250 = 1500 microseconds, number of retries = 15

  // Open a writing pipe
  radio.openWritingPipe(addresses[0]);

  radio.stopListening();                          // Stop listening so we can talk.
  radio.printDetails();           // Dump the configuration of the rf unit for debugging

  Wire.begin();
  // initialize sensor with normal i2c-address
  if (sht.init()) {
    Serial.print("init(): success\n");
  } else {
    Serial.print("init(): failed\n");
  }

  sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH); // only supported by SHT3x
}

void loop() {
  if (sht.readSample()) {
    // Send out the measured data
    // The payload
    Payload payload;
    payload.temperature = sht.getTemperature();
    payload.humidity = sht.getHumidity();

    // Serialise payloadss
    char serialisedPayload[sizeof(payload)];
    memcpy(serialisedPayload, &payload, sizeof(payload));

    // Send the payload
    Serial.print("Sending payload, size: ");
    Serial.print(sizeof(serialisedPayload));
    Serial.print(" temperature: ");
    Serial.print(payload.temperature);
    Serial.print(" humidity: ");
    Serial.print(payload.humidity);
    Serial.println("");

    radio.powerUp();
    radio.write(serialisedPayload, sizeof(serialisedPayload));   // This will block until complete
    radio.powerDown();
  } else {
    Serial.println("Sensor: Error in readSample()");
  }

  delay(5000);                                    // Try again 5s later

} // loop
