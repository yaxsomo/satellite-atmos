/* --- FULL MODIFIED ms5607.c --- */
#include <drivers_h/ms5607.h>
#include "tools_h/configuration.h"
#include <stdio.h>
#include <math.h>

/* SPI Transmission Data */
static uint8_t SPITransmitData;

/* Private OSR Instantiations */
static uint8_t Pressure_OSR =  OSR_256;
static uint8_t Temperature_OSR =  OSR_256;

/* PROM data structure */
static struct promData promData;
static struct MS5607UncompensatedValues uncompValues;
static struct MS5607Readings readings;

float initial_ms5607_pressure = 0.0;
float initial_ms5607_altitude = 0.0;

/* Kalman filter variables */
static float kalman_q = 4.0001f;
static float kalman_r = 0.20001f;
static float kalman_x = 0;
static float kalman_p = 0;
static float kalman_x_last = 0;
static float kalman_p_last = 0;

float kalman_filter(float altitude_measurement) {
    float x_temp = kalman_x_last;
    float p_temp = kalman_p_last + kalman_r;
    float k = p_temp / (p_temp + kalman_q);

    kalman_x = x_temp + k * (altitude_measurement - x_temp);
    kalman_p = (1 - k) * p_temp;

    kalman_x_last = kalman_x;
    kalman_p_last = kalman_p;

    return kalman_x;
}

float calculate_altitude(double pressure) {
    float altitude = 44330.0f * (1.0f - pow(pressure / initial_ms5607_pressure, 0.1903f));
    return altitude < 0 ? 0 : altitude;
}

void MS5607PromRead(struct promData *prom) {
    uint8_t address;
    uint16_t *structPointer = (uint16_t *) prom;

    for (address = 0; address < 8; address++) {
        SPITransmitData = PROM_READ(address);
        enableCSB();
        HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
        HAL_SPI_Receive(&hspi1, (uint8_t *)structPointer, 2, 10);
        disableCSB();
        structPointer++;
    }

    structPointer = (uint16_t *) prom;
    for (address = 0; address < 8; address++) {
        uint8_t *toSwap = (uint8_t *) structPointer;
        uint8_t secondByte = toSwap[0];
        toSwap[0] = toSwap[1];
        toSwap[1] = secondByte;
        structPointer++;
    }
}

void MS5607UncompensatedRead(struct MS5607UncompensatedValues *uncompValues) {
    uint8_t reply[3];

    enableCSB();
    SPITransmitData = CONVERT_D1_COMMAND | Pressure_OSR;
    HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
    while (hspi1.State == HAL_SPI_STATE_BUSY);
    HAL_Delay(10);
    disableCSB();

    enableCSB();
    SPITransmitData = READ_ADC_COMMAND;
    HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
    HAL_SPI_Receive(&hspi1, reply, 3, 10);
    disableCSB();
    uncompValues->pressure = ((uint32_t)reply[0] << 16) | ((uint32_t)reply[1] << 8) | reply[2];

    enableCSB();
    SPITransmitData = CONVERT_D2_COMMAND | Temperature_OSR;
    HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
    HAL_Delay(10);
    disableCSB();

    enableCSB();
    SPITransmitData = READ_ADC_COMMAND;
    HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
    HAL_SPI_Receive(&hspi1, reply, 3, 10);
    disableCSB();
    uncompValues->temperature = ((uint32_t)reply[0] << 16) | ((uint32_t)reply[1] << 8) | reply[2];
}

void MS5607Convert(struct MS5607UncompensatedValues *sample, struct MS5607Readings *value) {
    int32_t dT = sample->temperature - ((int32_t)(promData.tref << 8));
    int32_t TEMP = 2000 + (((int64_t)dT * promData.tempsens) >> 23);
    int64_t OFF = ((int64_t)promData.off << 17) + (((int64_t)promData.tco * dT) >> 6);
    int64_t SENS = ((int64_t)promData.sens << 16) + (((int64_t)promData.tcs * dT) >> 7);

    if (TEMP < 2000) {
        int32_t T2 = ((int64_t)dT * dT) >> 31;
        int32_t TEMPM = TEMP - 2000;
        int64_t OFF2 = (61 * (int64_t)TEMPM * TEMPM) >> 4;
        int64_t SENS2 = 2 * (int64_t)TEMPM * TEMPM;
        if (TEMP < -1500) {
            int32_t TEMPP = TEMP + 1500;
            OFF2 += (int64_t)15 * TEMPP * TEMPP;
            SENS2 += (int64_t)8 * TEMPP * TEMPP;
        }
        TEMP -= T2;
        OFF -= OFF2;
        SENS -= SENS2;
    }

    value->pressure = ((((int64_t)sample->pressure * SENS) >> 21) - OFF) >> 15;
    value->temperature = TEMP;
}

void MS5607Update(void) {
    MS5607UncompensatedRead(&uncompValues);
    MS5607Convert(&uncompValues, &readings);
}

float MS5607GetTemperatureC(void) {
    return (float)readings.temperature / 100.0f;
}

float MS5607GetPressurePa(void) {
    return (float)readings.pressure;
}

void enableCSB(void) {
    HAL_GPIO_WritePin(BARO_CS_GPIO_Port, BARO_CS_Pin, GPIO_PIN_RESET);
}

void disableCSB(void) {
    HAL_GPIO_WritePin(BARO_CS_GPIO_Port, BARO_CS_Pin, GPIO_PIN_SET);
}

void MS5607SetTemperatureOSR(MS5607OSRFactors tOSR) {
    Temperature_OSR = tOSR;
}

void MS5607SetPressureOSR(MS5607OSRFactors pOSR) {
    Pressure_OSR = pOSR;
}

Barometer_2_Axis MS5607_ReadData() {
    Barometer_2_Axis data = {0};

        MS5607Update();
        data.temperature = MS5607GetTemperatureC();
        data.pressure = MS5607GetPressurePa();
        float raw_altitude = calculate_altitude(data.pressure);
        data.altitude = kalman_filter(raw_altitude);


    return data;
}

float get_average_altitude() {
    float sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        MS5607Update();
        //float temp = MS5607GetTemperatureC();
        float press = MS5607GetPressurePa();
        float alt = calculate_altitude(press);
        sum += alt;
        HAL_Delay(10);
    }
    return sum / NUM_SAMPLES;
}

float get_average_pressure() {
    float sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        MS5607Update();
        sum += MS5607GetPressurePa();
        HAL_Delay(10);
    }
    return sum / NUM_SAMPLES;
}

void ms5607_print_barometer_data(Barometer_2_Axis *data) {
    printf("MS5607 Barometer:\n");
    printf("Pressure: %.3f Pa, Temperature: %.3f degC, Altitude: %.3f meters\n", data->pressure, data->temperature, data->altitude);
    printf("-----\n");
}

int8_t MS5607_Init() {
    enableCSB();
    SPITransmitData = RESET_COMMAND;
    HAL_SPI_Transmit(&hspi1, &SPITransmitData, 1, 10);
    while (hspi1.State == HAL_SPI_STATE_BUSY);
    HAL_Delay(3);
    disableCSB();

    MS5607PromRead(&promData);

    if (promData.reserved == 0x00 || promData.reserved == 0xff) {
        return MS5607_STATE_FAILED;
    } else {
        MS5607SetPressureOSR(OSR_4096);
        MS5607SetTemperatureOSR(OSR_4096);
        initial_ms5607_pressure = get_average_pressure();
        return MS5607_STATE_READY;
    }
}
