/**
 * Copyright (C) 2021 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include "Arduino.h"
#include "bme68xLibrary.h"

#define CS_PIN 5         // Chip select pin for the slave device
#define SPI_READ_CMD 0x80  // Read command for SPI (bit 7 = 1)

#ifndef PIN_CS
#define PIN_CS SS
#endif

Bme68x bme;

/**
 * @brief Initializes the sensor and hardware settings
 */
void setup(void)
{
	SPI.begin();
	Serial.begin(115200);
	
	while (!Serial)
		delay(10);
		
	/* initializes the sensor based on SPI library */
	bme.begin(PIN_CS, SPI);

	if(bme.checkStatus())
	{
		if (bme.checkStatus() == BME68X_ERROR)
		{
			Serial.println("Sensor error:" + bme.statusString());
			return;
		}
		else if (bme.checkStatus() == BME68X_WARNING)
		{
			Serial.println("Sensor Warning:" + bme.statusString());
		}
	}
	
	/* Set the default configuration for temperature, pressure and humidity */
	bme.setTPH();

	/* Set the heater configuration to 300 deg C for 100ms for Forced mode */
	bme.setHeaterProf(300, 100);

	Serial.println("TimeStamp(ms), Temperature(deg C), Pressure(Pa), Humidity(%), Gas resistance(ohm), Status");
}

void loop(void)
{
	bme68xData data;

	bme.setOpMode(BME68X_FORCED_MODE);
	delayMicroseconds(bme.getMeasDur());

	if (bme.fetchData())
	{
		bme.getData(data);
		Serial.print(String(millis()) + ", ");
		Serial.print(String(data.temperature) + ", ");
		Serial.print(String(data.pressure) + ", ");
		Serial.print(String(data.humidity) + ", ");
		Serial.print(String(data.gas_resistance) + ", ");
		Serial.println(data.status, HEX);
	}

    uint8_t data_1D = gas_meas_index(0x1D);
    Serial.print("Data read from register 0x1D: 0x");
    Serial.println(data_1D, HEX);
    Serial.print("\n");
    delay(1000);
}


uint8_t gas_meas_index(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | 0x80; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    uint8_t data = SPI.transfer(0x00); // Receive data from the register

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high

    return data;               // Return the read data
}