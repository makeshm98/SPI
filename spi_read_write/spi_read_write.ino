#include <SPI.h>

#define CS_PIN 5         // Chip select pin for the slave device
#define SPI_READ_CMD 0x80  // Read command for SPI (bit 7 = 1)

void setup() {
    Serial.begin(115200);   // Initialize serial communication
    pinMode(CS_PIN, OUTPUT); // Set chip select pin as output
    SPI.begin();           // Initialize SPI communication

    // Example: Write value 0x01 to the register at address 0x73
    writeRegister(0x73, 0x01);
    Serial.println("Value 0x01 written to register 0x73.");

    // Example: Read from the register at address 0x73
    uint8_t data = readRegister(0x73);
    Serial.print("Data read from register 0x73: 0x");
    Serial.println(data, HEX);  // Print the read data in hexadecimal format
}

void loop() {
    // Nothing to do in the loop
}

uint8_t readRegister(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | SPI_READ_CMD; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    uint8_t data = SPI.transfer(0x00); // Receive data from the register

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high

    return data;               // Return the read data
}

void writeRegister(uint8_t regAddress, uint8_t value) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    SPI.transfer(regAddress); // Send register address
    SPI.transfer(value);      // Send value to write to the register

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high
}
