#ifndef __ENS160_H
#define __ENS160_H

#include "stm32l4xx_hal.h" // Adjust if you use another STM32 series!
#include <stdint.h>

// ===== USER: SET YOUR I2C HANDLE HERE =====
extern I2C_HandleTypeDef hi2c3;
#define ENS160_I2C_HANDLE hi2c3

// ====== ENS160 I2C Address ======
#define ENS160_I2C_ADDR_DEFAULT    (0x53 << 1)   // 8-bit STM32 HAL format

// ====== ENS160 Register Map ======
#define ENS160_REG_PART_ID         0x00
#define ENS160_REG_OPMODE          0x10
#define ENS160_REG_CONFIG          0x11
#define ENS160_REG_COMMAND         0x12
#define ENS160_REG_TEMP_IN         0x13
#define ENS160_REG_RH_IN           0x15
#define ENS160_REG_DATA_STATUS     0x20
#define ENS160_REG_AQI             0x21
#define ENS160_REG_TVOC            0x22
#define ENS160_REG_ECO2            0x24
#define ENS160_REG_STATUS          0x2A
#define ENS160_REG_GPR_READ        0x30
#define ENS160_REG_GPR_WRITE       0x38
// (Add others if needed, this covers most key registers.)

// ====== ENS160 Operation Modes ======
#define ENS160_OPMODE_RESET        0xF0
#define ENS160_OPMODE_SLEEP        0x00
#define ENS160_OPMODE_IDLE         0x01
#define ENS160_OPMODE_STD          0x02

// ====== ENS160 Return Codes ======
#define ENS160_OK                  0
#define ENS160_ERROR               1

// ====== ENS160 AQI Classes ======
typedef enum {
    ENS160_AQI_INVALID = 0,
    ENS160_AQI_EXCELLENT = 1,
    ENS160_AQI_GOOD = 2,
    ENS160_AQI_MODERATE = 3,
    ENS160_AQI_POOR = 4,
    ENS160_AQI_UNHEALTHY = 5
} ens160_aqi_t;

// ====== ENS160 Data Struct ======
typedef struct {
    I2C_HandleTypeDef *i2c;
    uint8_t address;
    uint8_t status;
    uint8_t aqi;
    uint16_t tvoc;
    uint16_t eco2;
} ENS160_t;

// ====== ENS160 API ======

// Basic device setup and checks
void     ENS160_Init(ENS160_t *dev);
uint8_t  ENS160_CheckDevice(ENS160_t *dev);
uint8_t  ENS160_GetPartID(ENS160_t *dev);

// Operation mode
uint8_t  ENS160_SetMode(ENS160_t *dev, uint8_t mode);

// Environmental inputs (for compensation)
uint8_t  ENS160_SetTempAndRH(ENS160_t *dev, float temp_celsius, float rh_percent);

// Data readout
uint8_t  ENS160_ReadData(ENS160_t *dev); // Updates dev->aqi, tvoc, eco2
uint8_t  ENS160_GetAQI(ENS160_t *dev, uint8_t *aqi);
uint8_t  ENS160_GetTVOC(ENS160_t *dev, uint16_t *tvoc);
uint8_t  ENS160_GetECO2(ENS160_t *dev, uint16_t *eco2);

// Advanced/extra
uint8_t  ENS160_SoftwareReset(ENS160_t *dev);
uint8_t  ENS160_GetStatus(ENS160_t *dev, uint8_t *status);

// Optional: Custom register access
uint8_t  ENS160_ReadRegister(ENS160_t *dev, uint8_t reg, uint8_t *data, uint16_t len);
uint8_t  ENS160_WriteRegister(ENS160_t *dev, uint8_t reg, uint8_t *data, uint16_t len);

#endif // __ENS160_H
