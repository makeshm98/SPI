#include <SPI.h>
int number = 0;
const int arraySize = 10;
float lastTempComp[arraySize];

#define CS_PIN 5         
#define SPI_READ_CMD 0x80  

uint8_t data_array_8a[23]; 
uint8_t data_array_e1[14]; 
uint8_t data_array_00[5];
uint8_t data_array_1d[17];
uint8_t data_array_71[7];

#define BME68X_RHRANGE_MSK                        UINT8_C(0x30)

#define BME68X_RSERROR_MSK                        UINT8_C(0xf0)

#ifndef BME68X_PERIOD_POLL
#define BME68X_PERIOD_POLL                        UINT32_C(10000)
#endif

/* Length of the field */
#define BME68X_LEN_FIELD                          UINT8_C(17)

#define BME68X_OSH_MSK                            UINT8_C(0X07)
#define BME68X_FILTER_MSK                         UINT8_C(0X1c)
#define BME68X_FILTER_POS                         UINT8_C(2)
#define BME68X_OST_MSK                            UINT8_C(0Xe0)
#define BME68X_OST_POS                            UINT8_C(5)
#define BME68X_OSP_MSK                            UINT8_C(0X1c)
#define BME68X_OSP_POS                            UINT8_C(2)

#define BME68X_GET_BITS(reg_data, bitname)        ((reg_data & (bitname##_MSK)) >> (bitname##_POS))
#define BME68X_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))

uint8_t buff[BME68X_LEN_FIELD] = { 0 };

void setup() {
    Serial.begin(115200);   
    pinMode(CS_PIN, OUTPUT); 
    SPI.begin();           
 
}

void loop() {
    writeRegister(0xE0, 0xb6); 

    writeRegister(0x73, 0b00010000);
    writeRegister(0x70, 0b00000000); 
    writeRegister(0x71, 0b00100101);  
    writeRegister(0x5A + 5, 0b01010011); 
    writeRegister(0x64 + 5, 0b01011001);
    writeRegister(0x72, 0b00000001);
    writeRegister(0x74, 0b00); 
    writeRegister(0x75, 0b00000100); 
    writeRegister(0x74, 0b01010101);

    delay(1000);

    
    readRegister(0x8a, data_array_8a, 23); 
    readRegister_00(0xe1, data_array_e1, 14); 
    readRegister_00(0x00, data_array_00, 5);

    writeRegister(0x73, 0b00010000);
    readRegister_02(0x1D, data_array_1d, 17);


    readRegister(0x71, data_array_71, 7);

    
    uint8_t os_hum = BME68X_GET_BITS_POS_0(data_array_71[1], BME68X_OSH);;
    uint8_t filter = BME68X_GET_BITS(data_array_71[4], BME68X_FILTER);
    uint8_t os_temp = BME68X_GET_BITS(data_array_71[3], BME68X_OST);
    uint8_t os_pres = BME68X_GET_BITS(data_array_71[3], BME68X_OSP);


    //Finding oversampling rate
    uint8_t os_to_meas_cycles[6] = { 0, 1, 2, 4, 8, 16 };
    uint32_t meas_cycles;
    meas_cycles = os_to_meas_cycles[os_temp];
    meas_cycles += os_to_meas_cycles[os_pres];
    meas_cycles += os_to_meas_cycles[os_hum];

    uint32_t meas_dur = 0;
    /* TPH measurement duration */
    meas_dur = meas_cycles * UINT32_C(1963);
    meas_dur += UINT32_C(477 * 4); /* TPH switching duration */
    meas_dur += UINT32_C(477 * 5); /* Gas measurement duration */

    Serial.print("Sensor measurement duration: ");
    Serial.print(meas_dur);
    Serial.println(" ");

    delayMicroseconds(meas_dur);

    uint8_t data_01 = data_array_8a[0];
    uint8_t data_02 = data_array_8a[1];
    uint8_t data_03 = data_array_8a[2];

    uint8_t data_04 = data_array_e1[8]; 
    uint8_t data_05 = data_array_e1[9]; 


    uint16_t par_t1 = (uint16_t)((data_05 << 8) | data_04);
    uint16_t par_t2 = (uint16_t)((data_01 << 8) | data_02);
    uint16_t par_t3 = data_03;


    uint16_t data_06 = readRegister_01(0x22);
    uint16_t data_07 = readRegister_01(0x23);

    float adc_temp = (uint32_t) (((uint32_t) data_array_1d[5] * 4096) | ((uint32_t) data_array_1d[6] * 16) | ((uint32_t) data_array_1d[7] / 16));


    float var1 = (((float)adc_temp / 16384.0f) - ((float)par_t1 / 1024.0f)) * (float)par_t2;
    float var2 = ((((float)adc_temp / 131072.0f) - ((float)par_t1 / 8192.0f)) *  (((float)adc_temp / 131072.0f) - ((float)par_t1 / 8192.0f))) * ((float)par_t3 * 16.0f);

    float t_fine = var1 + var2;
    float temp_comp = t_fine / 5120.0f ;

    Serial.print("Compensated temperature value in degree celsius : ");
    Serial.println(temp_comp, DEC);

    Serial.println(" ");

    //Temperature calculation end 


    //Pressure calculation starts 
    float var1_pressure;
    float var2_pressure;
    float var3;
    float calc_pres;
    float pres_adc;

    uint8_t data_p5 = data_array_8a[12]; //0x96
    uint8_t data_p6 = data_array_8a[13]; //0x97
    uint8_t data_p3 = data_array_8a[10]; //0x94
    uint8_t data_p4 = data_array_8a[11]; //0x95
    uint8_t data_92 = data_array_8a[8]; //0x92
    uint8_t data_p2_lsb = data_array_8a[6]; //0x90
    uint8_t data_p2_msb = data_array_8a[7]; //0x91
    uint8_t data_p1_lsb = data_array_8a[4];  //0x8E
    uint8_t data_p1_msb = data_array_8a[5]; //0x8F 
    uint8_t data_p9_lsb = data_array_8a[20]; //0x9E
    uint8_t data_p9_msb = data_array_8a[21]; //0x9F
    uint8_t data_p8_lsb = data_array_8a[18]; //0x9C
    uint8_t data_p8_msb = data_array_8a[19]; //0x9D
    uint8_t data_p7 = data_array_8a[14]; //0x98
    uint8_t data_p10 = data_array_8a[22]; //0xA0

    uint16_t par_p5 = (uint16_t)((data_p6 << 8) | data_p5);
    uint16_t par_p4 = (uint16_t)((data_p4 << 8) | data_p3);
    uint16_t par_p2 = (uint16_t)((data_p2_msb << 8) | data_p2_lsb);
    uint16_t par_p1 = (uint16_t)((data_p1_msb << 8) | data_p1_lsb);
    uint16_t par_p9 = (uint16_t)((data_p9_msb << 8) | data_p9_lsb);
    uint16_t par_p8 = (uint16_t)((data_p8_msb << 8) | data_p8_lsb);

    pres_adc = (uint32_t)(((uint32_t)data_array_1d[2] * 4096) | ((uint32_t)data_array_1d[3] * 16) | ((uint32_t)data_array_1d[4] / 16));

    var1_pressure = (((float)(t_fine) / 2.0f) - 64000.0f);
    var2_pressure = var1_pressure * var1_pressure * (((float)data_array_8a[15]) / (131072.0f));
    var2_pressure = var2_pressure + (var1_pressure * ((float)par_p5) * 2.0f);
    var2_pressure = (var2_pressure / 4.0f) + (((float)par_p4) * 65536.0f);
    var1_pressure = (((((float)data_92 * var1_pressure * var1_pressure) / 16384.0f) + ((float)par_p2 * var1_pressure)) / 524288.0f);
    var1_pressure = ((1.0f + (var1_pressure / 32768.0f)) * ((float)par_p1));
    calc_pres = (1048576.0f - ((float)pres_adc));

    /* Avoid exception caused by division by zero */
    if ((int)var1 != 0)
    {
        calc_pres = (((calc_pres - (var2_pressure / 4096.0f)) * 6250.0f) / var1_pressure);
        var1_pressure = (((float)par_p9) * calc_pres * calc_pres) / 2147483648.0f;
        var2_pressure = calc_pres * (((float)par_p8) / 32768.0f);
        var3 = ((calc_pres / 256.0f) * (calc_pres / 256.0f) * (calc_pres / 256.0f) * (data_p10 / 131072.0f));
        calc_pres = (calc_pres + (var1 + var2_pressure + var3 + ((float)data_p7 * 128.0f)) / 16.0f);
    }
    else
    {
        calc_pres = 0;
    }
    Serial.print("Compensated pressure value in pascal : ");
    Serial.println(calc_pres, DEC);


    uint8_t data_08 = data_array_e1[12];
    uint8_t data_09 = data_array_e1[11];
    uint8_t data_10 = data_array_e1[10];
    uint8_t data_11 = data_array_e1[13];


    uint8_t data_12 = data_array_00[2];
    uint8_t data_14 = data_array_00[0];
    uint8_t data_15 = data_array_00[4];

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

    //delay(1000);

    int size = 17;
    float humidity = humidity_01(temp_comp, data_array_1d, size);
    Serial.print("Humidity in %RH : ");
    Serial.println(humidity, DEC);

    uint8_t data = gas_meas_index(0x1D);
    Serial.print("Data read from register 0x1D: 0x");
    Serial.println(data, HEX);
    Serial.print("\n");

    uint16_t heat = gas_meas_index(0x2D);
    Serial.print("Data read from register 0x2D: 0x");
    Serial.println(heat, HEX);
    Serial.print("\n");  

    uint16_t gas_adc = (uint16_t) ((uint32_t) data_array_1d[15] * 4 | (((uint32_t) data_array_1d[16]) / 64));
    uint16_t gas_range = data_array_1d[16] & 0b00001111;

    float calc_gas_res;
    uint32_t var1_gas_01 = UINT32_C(262144) >> gas_range;
    int32_t var2_gas_01 = (int32_t)gas_adc - INT32_C(512);
    var2_gas_01 *= INT32_C(3);    
    var2_gas_01 = INT32_C(4096) + var2_gas_01;
    calc_gas_res = 1000000.0f * (float)var1_gas_01 / (float)var2_gas_01;
    
    Serial.print("Compensated Gas Resistance value in Ohm : ");
    Serial.println(calc_gas_res, DEC);
    //Gas measurements ended  

    // Example: Read from the register at address 0x70
    uint8_t data_70 = readRegister_70(0x70);
    Serial.print("Data read from register 0x70 heater: 0x");
    Serial.println(data_70, HEX); 

    uint8_t data_71 = readRegister_71(0x70);
    Serial.print("Data read from register 0x70 variant_ID: 0x");
    Serial.println(data_71, HEX);   


    uint8_t data_5F = heat_5F(0x5A + 5);
    Serial.print("Data read from register 0x5F: 0x");
    Serial.println(data_5F, HEX);
    Serial.print("\n");
}

uint8_t heat_5F(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | 0x80; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    uint8_t data = SPI.transfer(0x00); // Receive data from the register

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high

    return data;               // Return the read data
}

uint8_t heat(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); // Select the slave device by bringing CSB low
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Start SPI transaction

    uint8_t controlByte = regAddress | 0x80; // Set the MSB for read operation
    SPI.transfer(controlByte); // Send control byte (register address with read command)
    uint8_t data = SPI.transfer(0x00); // Receive data from the register

    SPI.endTransaction();      // End SPI transaction
    digitalWrite(CS_PIN, HIGH); // Deselect the slave device by bringing CSB high

    return data;               // Return the read data
}

uint8_t readRegister_70(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | 0x80; 
    SPI.transfer(controlByte); 
    uint8_t data = SPI.transfer(0x00); 

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH); 

    return data;               
}

uint8_t readRegister_71(uint8_t regAddress) {
    writeRegister(0x73, 0b00000000);
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | 0x80; 
    SPI.transfer(controlByte); 
    uint8_t data = SPI.transfer(0x00); 

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH); 

    return data;               
}

float humidity_01(float temp_comp, uint8_t data_array_1d[], int size)
{
  float hum_adc = (uint16_t)(((uint32_t) data_array_1d[8] * 256) | (uint32_t)data_array_1d[9]);

  float par_h1 = (uint16_t)(((uint16_t) data_array_e1[2] << 4) | data_array_e1[1] & 0x0F);
  float par_h2 = (uint16_t)(((uint16_t) data_array_e1[0] << 4) | data_array_e1[1] >> 4);
  float par_h3 = (int8_t) data_array_e1[3];
  float par_h4 = (int8_t) data_array_e1[4];
  float par_h5 = (int8_t) data_array_e1[5];
  float par_h6 = (int8_t) data_array_e1[6];
  float par_h7 = (int8_t) data_array_e1[7];
  
  

  float var1_hum = hum_adc - (((double)par_h1 * 16.0) + (((double)par_h3 / 2.0) * temp_comp));
  float var2_hum = var1_hum * (((double)par_h2 / 262144.0) * (1.0 + (((double)par_h4 / 16384.0) * temp_comp) + (((double)par_h5 / 1048576.0) * temp_comp * temp_comp)));
  float var3_hum = (double)par_h6 / 16384.0;
  float var4_hum = (double)par_h7 / 2097152.0;
  float hum_comp = var2_hum + ((var3_hum + (var4_hum * temp_comp)) * var2_hum * var2_hum);
  if (hum_comp > 100.0f)
  {
    hum_comp = 100.0f;
  }
  else if (hum_comp < 0.0f)
  {
    hum_comp = 0.0f;
  }

  return hum_comp;
}

uint16_t readRegister_01(uint16_t regAddress)
{
  writeRegister(0x73, 0b00010000);
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  uint16_t controlByte = regAddress | SPI_READ_CMD;
  SPI.transfer(controlByte);
  uint16_t data = SPI.transfer(0x00);
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);

  return data;
  writeRegister(0x73, 0b00000000);

}

void writeRegister(uint8_t regAddress, uint8_t data) {
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress & 0x7F; 
    SPI.transfer(controlByte); 
    SPI.transfer(data);        

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH);

    Serial.print("Data written to register 0x");
    Serial.print(regAddress, HEX);
    Serial.print(": 0x");
    Serial.println(data, HEX);  
}

void readRegister_00(uint8_t regAddress, uint8_t *data, uint8_t length) {
    writeRegister(0x73, 0b00000000);
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | SPI_READ_CMD; 
    SPI.transfer(controlByte); 
    for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00); 
    }

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH); 
    writeRegister(0x73, 0b00010000);

}


void readRegister(uint8_t regAddress, uint8_t *data, uint8_t length) {
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | SPI_READ_CMD; 
    SPI.transfer(controlByte); 
    for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00); 
    }

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH); 
}

static uint8_t calc_heatr_dur_shared(uint16_t dur)
{
    uint8_t factor = 0;
    uint8_t heatdurval;

    if (dur >= 0x783)
    {
        heatdurval = 0xff; /* Max duration */
    }
    else
    {
        /* Step size of 0.477ms */
        dur = (uint16_t)(((uint32_t)dur * 1000) / 477);
        while (dur > 0x3F)
        {
            dur = dur >> 2;
            factor += 1;
        }

        heatdurval = (uint8_t)(dur + (factor * 64));
    }

    return heatdurval;
    delay(1000);

}

static uint8_t calc_res_heat(uint16_t temp)
{

    uint8_t data_08 = data_array_e1[12]; 
    uint8_t data_09 = data_array_e1[11]; 
    uint8_t data_10 = data_array_e1[10]; 
    uint8_t data_11 = data_array_e1[13]; 

    uint8_t data_12 = data_array_00[2]; 
    uint8_t data_14 = data_array_00[0]; 

    uint8_t res_heat_range = (data_12 & BME68X_RHRANGE_MSK) / 16;
    uint8_t res_heat_val = data_14;

    uint8_t par_g1 = data_08;
    uint8_t par_g2 = data_09 << 8 | data_10;
    uint8_t par_g3 = data_11;

    int amb_temp = 25;

    float var1_gas = ((float)par_g1 / 16.0) + 49.0;
    float var2_gas = (((float)par_g2 / 32768.0) * 0.0005) + 0.00235;
    float var3_gas = (float)par_g3 / 1024.0;
    float var4_gas = var1_gas * (1.0 + (var2_gas * (float) temp));
    float var5_gas = var4_gas + (var3_gas * (float)amb_temp);

    uint8_t res_heat = (uint8_t)(3.4f * ((var5_gas * (4.0 / (4.0 + (float)res_heat_range)) * (1.0/(1.0 +
((float)res_heat_val * 0.002f)))) - 25));

    return res_heat;

    Serial.print("By function : ");
    Serial.println(res_heat, DEC);

    Serial.println(" ");
    delay(1000);

}


void readRegister_02(uint8_t regAddress, uint8_t *data, uint8_t length) {
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | SPI_READ_CMD; 
    SPI.transfer(controlByte); 
    for (int i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00); 
    }
    SPI.endTransaction();     
    digitalWrite(CS_PIN, HIGH); 
}

uint8_t gas_meas_index(uint8_t regAddress) {
    digitalWrite(CS_PIN, LOW); 
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); 

    uint8_t controlByte = regAddress | 0x80; 
    SPI.transfer(controlByte); 
    uint8_t data = SPI.transfer(0x00); 

    SPI.endTransaction();      
    digitalWrite(CS_PIN, HIGH); 

    return data;               
}