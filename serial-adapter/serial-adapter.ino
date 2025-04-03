#include <SPI.h>
#include "printf.h"
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pins

// Address matching the transmitter
const uint64_t address = 0xF0F0F0F001LL;

// Define the payload structure to match the sender
struct Payload {
    uint8_t deviceId;
    float temperature;
    float humidity;
    uint16_t moisture;
};

void setup() {
    Serial.begin(115200);
    printf_begin();  // needed for printDetails()
    Serial.println("Starting receiver...");

    // Initialize radio
    if (!radio.begin()) {
        Serial.println("nRF24L01+ not responding");
        while (1); // Halt execution
    }

    radio.setRetries(5, 15); // 5 * 250µs delay between retries, max 15 retries
    //radio.setPALevel(RF24_PA_LOW); // Lower power level to avoid noise
    //radio.setDataRate(RF24_250KBPS); // Lower data rate = Better reliability
    radio.setChannel(76); // default 
    //radio.setCRCLength(RF24_CRC_16);
    //radio.setAddressWidth(5); // 5-byte address
    
    // Open reading pipe to match the sender address
    radio.openReadingPipe(0, address);
    radio.enableDynamicPayloads(); // Allow flexible payload size
    radio.setAutoAck(true); // Enable ACK for reliable transmission
    
    radio.printDetails(); // Print radio configuration for debugging
    radio.startListening(); // Start receiving mode

    Serial.println("Receiver ready!");
}

void loop() {
    if (radio.available()) {
        Payload payload;
        uint8_t len = radio.getDynamicPayloadSize(); // Read payload size

        if (len == sizeof(Payload)) {
            radio.read(&payload, len);

            // Print received data to serial monitor
            Serial.print("Received - Temp: ");
            Serial.print(payload.temperature);
            Serial.print(" °C, Humidity: ");
            Serial.print(payload.humidity);
            Serial.print(" %, Moisture: ");
            Serial.print(payload.moisture);
            Serial.println();

            // Send back an ACK payload (optional)
            Payload ackPayload;
            ackPayload.temperature = 21.5;
            ackPayload.humidity = 50.0;
            ackPayload.moisture = 400;

            // Write ACK payload — this will be sent with the ACK
            radio.writeAckPayload(0, &ackPayload, sizeof(ackPayload));
        } else {
            Serial.print("Invalid payload size: ");
            Serial.println(len);
        }
    }
}
