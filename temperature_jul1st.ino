#include <SPI.h>

#define CS_PIN 5         // Chip select pin for the slave device
#define SPI_READ_CMD 0x80  // Read command for SPI (bit 7 = 1)

uint8_t data_array_8a[23]; // Array to store the response from register 0x8a
uint8_t data_array_e1[14]; // Array to store the response from register 0xe1
uint8_t data_array_00[5];
uint8_t data_array_1d[17];

/* Mask for res heat range */
#define BME68X_RHRANGE_MSK                        UINT8_C(0x30)

/* Mask for range switching error */
#define BME68X_RSERROR_MSK                        UINT8_C(0xf0)

/* Period between two polls (value can be given by user) */
#ifndef BME68X_PERIOD_POLL
#define BME68X_PERIOD_POLL                        UINT32_C(10000)
#endif

/* Length of the field */
#define BME68X_LEN_FIELD                          UINT8_C(17)

uint8_t buff[BME68X_LEN_FIELD] = { 0 };

void setup() {
    Serial.begin(115200);   // Initialize serial communication
    pinMode(CS_PIN, OUTPUT); // Set chip select pin as output
    SPI.begin();           // Initialize SPI communication
}

void loop() {
    writeRegister(0xE0, 0xb6); //Board reset 
    writeRegister(0x73, 0b00000000);//zeroth page selection
    writeRegister(0x74, 0b00000001);//forced mode trigger for zeroth page

    writeRegister(0x73, 0b00010000);//memory page 1 selection
    writeRegister(0x72, 0b00000001);//humidity oversampling
    writeRegister(0x75, 0b00000100); //after applying this filter register value just get normalized fluctuations 
    writeRegister(0x74, 0b00);
    writeRegister(0x74, 0b01010101);//temperature and humidity oversampling
    writeRegister(0x74, 0b00000001);//forced mode trigger

    writeRegister(0x5A + 5, 0b00100100); //assigning target heater resistance value for gas sensor hot plate to register 0x5A
    writeRegister(0x64 + 5, 0b01011001); //100ms for heating duration
    //writeRegister(0x70, 0b00000000); //heater on
    writeRegister(0x71, 0b00100101); //previously defined heater settings and run gas measurements 

    readRegister(0x8a, data_array_8a, 23); // Read from register 0x8a and store response in data_array_8a
    readRegister(0xe1, data_array_e1, 14); // Read from register 0xe1 and store response in data_array_e1
    readRegister(0x00, data_array_00, 5);

    //writeRegister(0x73, 0b00010000);//memory page 1 selection
    readRegister_02(0x1D, data_array_1d, 17);

    Serial.println("Data read from register 0x8a:");
    for (int i = 0; i < 23; i++) {
        Serial.print("0x");
        Serial.print(data_array_8a[i], HEX); // Print each element of the data array in hexadecimal format
        Serial.print(" ");
    }
    Serial.println();

    Serial.println("Data read from register 0xe1:");
    for (int i = 0; i < 14; i++) {
        Serial.print("0x");
        Serial.print(data_array_e1[i], HEX); // Print each element of the data array in hexadecimal format
        Serial.print(" ");
    }
    Serial.println();

    writeRegister(0x73, 0b00010000);
    Serial.println("Data read from register 0x00:");
    for (int i = 0; i < 5; i++) {
        Serial.print("0x");
        Serial.print(data_array_00[i], HEX); // Print each element of the data array in hexadecimal format
        Serial.print(" ");
    }
    Serial.println();
    writeRegister(0x73, 0b00010000);
    Serial.println("Data read from register 0x1d:");
    for (int i = 0; i < 17; i++) {
        Serial.print("0x");
        Serial.print(data_array_1d[i], HEX); // Print each element of the data array in hexadecimal format
        Serial.print(" ");
    }
    Serial.println();

    uint8_t data_01 = data_array_8a[0];
    uint8_t data_02 = data_array_8a[1];
    uint8_t data_03 = data_array_8a[2];

    uint8_t data_04 = data_array_e1[8];
    uint8_t data_05 = data_array_e1[9];


    uint16_t par_t1 = (uint16_t)((data_04 << 8) | data_05);
    uint16_t par_t2 = (uint16_t)((data_02 << 8) | data_01);
    uint16_t par_t3 = data_03;


    uint16_t data_06 = readRegister_01(0x22);
    uint16_t data_07 = readRegister_01(0x23);

    uint32_t adc_temp;
    adc_temp = (uint32_t)(((uint32_t)data_array_1d[5] * 4096) | ((uint32_t)data_array_1d[6] * 16) | ((uint32_t)data_array_1d[7] / 16));



    //long temp_adc = data_06 << 8 | data_07; 

    long var1 = (((double)adc_temp / 16384.0) - ((double)par_t1 / 1024.0)) * (double)par_t2;
    long var2 = ((((double)adc_temp / 131072.0) - ((double)par_t1 / 8192.0)) *  (((double)adc_temp / 131072.0) - ((double)par_t1 / 8192.0))) * ((double)par_t3 * 16.0);

    long t_fine = var1 + var2;
    float temp_comp = t_fine / 5120.0 ;

    Serial.print("Compensated temperature value in degree celsius : ");
    Serial.println(temp_comp, DEC);

    Serial.println(" ");
    delay(1000);

    uint8_t data_08 = data_array_e1[12];
    uint8_t data_09 = data_array_e1[11];
    uint8_t data_10 = data_array_e1[10];
    uint8_t data_11 = data_array_e1[13];


    uint8_t data_12 = data_array_00[2];
    //uint8_t data_13 = data_array_e1[7];
    uint8_t data_14 = data_array_00[0];
    uint8_t data_15 = data_array_00[4];
    //uint8_t data_16 = data_array_e1[f0];

    int amb_temp = 25;

    uint8_t par_g1 = data_08;
    uint8_t par_g2 = data_09 << 8 | data_10;
    uint8_t par_g3 = data_11;

    uint8_t res_heat_range = (data_12 & BME68X_RHRANGE_MSK) / 16;
    uint8_t range_sw_err = (data_15 & BME68X_RSERROR_MSK) / 16;
    uint8_t res_heat_val = data_14;

    long var1_gas = ((double)par_g1 / 16.0) + 49.0;
    long var2_gas = (((double)par_g2 / 32768.0) * 0.0005) + 0.00235;
    long var3_gas = (double)par_g3 / 1024.0;
    long var4_gas = var1_gas * (1.0 + (var2_gas * (double) 300));
    long var5_gas = var4_gas + (var3_gas * (double)amb_temp);

    long res_heat_x = (uint8_t)(3.4 * ((var5_gas * (4.0 / (4.0 + (double)res_heat_range)) * (1.0/(1.0 +
((double)res_heat_val * 0.002)))) - 25));

    Serial.print("Compensated resistance value needed for 300 degree celsius : ");
    Serial.println(res_heat_x, DEC);

    Serial.println(" ");

    delay(1000);
}

uint16_t readRegister_01(uint16_t regAddress)
{
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  uint16_t controlByte = regAddress | SPI_READ_CMD;
  SPI.transfer(controlByte);
  uint16_t data = SPI.transfer(0x00);
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);

  return data;
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

void readRegister(uint8_t regAddress, uint8_t *data, uint8_t length) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | SPI_READ_CMD; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00); // Receive data from the register and store in data array
    }

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high
}

void readRegister_02(uint8_t regAddress, uint8_t *data, uint8_t length) {
    writeRegister(0xE0, 0xb6); //Board reset 
    writeRegister(0x73, 0b00000000);//zeroth page selection
    writeRegister(0x74, 0b00000001);//forced mode trigger for zeroth page

    writeRegister(0x73, 0b00010000);//memory page 1 selection
    writeRegister(0x72, 0b00000001);//humidity oversampling
    writeRegister(0x75, 0b00000100); //after applying this filter register value just get normalized fluctuations 
    writeRegister(0x74, 0b00);
    writeRegister(0x74, 0b01010101);//temperature and humidity oversampling
    writeRegister(0x74, 0b00000001);//forced mode trigger

    writeRegister(0x5A + 5, 0b00100100); //assigning target heater resistance value for gas sensor hot plate to register 0x5A
    writeRegister(0x64 + 5, 0b01011001); //100ms for heating duration
    //writeRegister(0x70, 0b00000000); //heater on
    writeRegister(0x71, 0b00100101); //previously defined heater settings and run gas measurements 

    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | SPI_READ_CMD; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00); // Receive data from the register and store in data array
    }

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high
}
