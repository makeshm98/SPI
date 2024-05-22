#include <SPI.h>

#define CS_PIN 5         // Chip select pin for the slave device
#define SPI_WRITE_CMD 0x00 // Write command for SPI (bit 7 = 0)

void setup() {
    Serial.begin(115200);   // Initialize serial communication
    pinMode(CS_PIN, OUTPUT); // Set chip select pin as output
    SPI.begin();           // Initialize SPI communication

    // Example: Write 0x01 to the register at address 0x73
    writeRegister(0x73, 0x01);
}

void loop() {
    // Nothing to do in the loop
}

void writeRegister(uint8_t regAddress, uint8_t data) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress & 0x7F; // Clear the MSB for write operation
    SPI.transfer(controlByte); // Send control byte (register address with write command)
    SPI.transfer(data);        // Send the data byte

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high

    Serial.print("Data written to register 0x");
    Serial.print(regAddress, HEX);
    Serial.print(": 0x");
    Serial.println(data, HEX);  // Print the written data in hexadecimal format
}
