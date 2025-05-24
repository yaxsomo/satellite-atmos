#include "drivers_h/ens160.h"

// ============ Low-Level I2C Read/Write Helpers ============

uint8_t ENS160_ReadRegister(ENS160_t *dev, uint8_t reg, uint8_t *data, uint16_t len) {
    if (HAL_I2C_Mem_Read(dev->i2c, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100) != HAL_OK)
        return ENS160_ERROR;
    return ENS160_OK;
}

uint8_t ENS160_WriteRegister(ENS160_t *dev, uint8_t reg, uint8_t *data, uint16_t len) {
    if (HAL_I2C_Mem_Write(dev->i2c, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100) != HAL_OK)
        return ENS160_ERROR;
    return ENS160_OK;
}

// ============ Public API ============

void ENS160_Init(ENS160_t *dev) {
    dev->i2c     = &ENS160_I2C_HANDLE;
    dev->address = ENS160_I2C_ADDR_DEFAULT;
    dev->status  = 0;
    dev->aqi     = 0;
    dev->tvoc    = 0;
    dev->eco2    = 0;
    // Optionally, perform a reset
    ENS160_SoftwareReset(dev);
    HAL_Delay(10);
}

uint8_t ENS160_CheckDevice(ENS160_t *dev) {
    uint8_t id = 0;
    if (ENS160_ReadRegister(dev, ENS160_REG_PART_ID, &id, 1) != ENS160_OK)
        return ENS160_ERROR;
    if (id == 0 || id == 0xFF) // invalid response
        return ENS160_ERROR;
    return ENS160_OK;
}

uint8_t ENS160_GetPartID(ENS160_t *dev) {
    uint8_t id = 0;
    ENS160_ReadRegister(dev, ENS160_REG_PART_ID, &id, 1);
    return id;
}

uint8_t ENS160_SoftwareReset(ENS160_t *dev) {
    uint8_t mode = ENS160_OPMODE_RESET;
    if (ENS160_WriteRegister(dev, ENS160_REG_OPMODE, &mode, 1) != ENS160_OK)
        return ENS160_ERROR;
    HAL_Delay(10); // Give it time to reset
    return ENS160_OK;
}

uint8_t ENS160_SetMode(ENS160_t *dev, uint8_t mode) {
    if (ENS160_WriteRegister(dev, ENS160_REG_OPMODE, &mode, 1) != ENS160_OK)
        return ENS160_ERROR;
    HAL_Delay(2); // Short delay for mode change
    return ENS160_OK;
}

uint8_t ENS160_SetTempAndRH(ENS160_t *dev, float temp_celsius, float rh_percent) {
    // Convert temperature to 16-bit value (see datasheet)
    // Formula: Temp_in = (T(°C) + 273.15) * 64 (result: 16-bit unsigned int, 0.1 K steps)
    //          RH_in   = RH (%) * 512 (result: 16-bit unsigned int, 0.001% steps)
    uint16_t t_raw = (uint16_t)(((temp_celsius + 273.15f) * 64.0f) + 0.5f);
    uint16_t rh_raw = (uint16_t)((rh_percent * 512.0f) + 0.5f);

    uint8_t t_buf[2] = { t_raw & 0xFF, (t_raw >> 8) & 0xFF };
    uint8_t rh_buf[2] = { rh_raw & 0xFF, (rh_raw >> 8) & 0xFF };

    if (ENS160_WriteRegister(dev, ENS160_REG_TEMP_IN, t_buf, 2) != ENS160_OK)
        return ENS160_ERROR;
    if (ENS160_WriteRegister(dev, ENS160_REG_RH_IN, rh_buf, 2) != ENS160_OK)
        return ENS160_ERROR;
    return ENS160_OK;
}

uint8_t ENS160_ReadData(ENS160_t *dev) {
    uint8_t status = 0, aqi = 0;
    uint8_t tvoc_buf[2] = {0}, eco2_buf[2] = {0};

    if (ENS160_ReadRegister(dev, ENS160_REG_DATA_STATUS, &status, 1) != ENS160_OK)
        return ENS160_ERROR;
    dev->status = status;

    if (ENS160_ReadRegister(dev, ENS160_REG_AQI, &aqi, 1) != ENS160_OK)
        return ENS160_ERROR;
    dev->aqi = aqi;

    if (ENS160_ReadRegister(dev, ENS160_REG_TVOC, tvoc_buf, 2) != ENS160_OK)
        return ENS160_ERROR;
    dev->tvoc = (uint16_t)tvoc_buf[0] | ((uint16_t)tvoc_buf[1] << 8);

    if (ENS160_ReadRegister(dev, ENS160_REG_ECO2, eco2_buf, 2) != ENS160_OK)
        return ENS160_ERROR;
    dev->eco2 = (uint16_t)eco2_buf[0] | ((uint16_t)eco2_buf[1] << 8);

    return ENS160_OK;
}

uint8_t ENS160_GetAQI(ENS160_t *dev, uint8_t *aqi) {
    if (dev == NULL || aqi == NULL)
        return ENS160_ERROR;
    *aqi = dev->aqi;
    return ENS160_OK;
}

uint8_t ENS160_GetTVOC(ENS160_t *dev, uint16_t *tvoc) {
    if (dev == NULL || tvoc == NULL)
        return ENS160_ERROR;
    *tvoc = dev->tvoc;
    return ENS160_OK;
}

uint8_t ENS160_GetECO2(ENS160_t *dev, uint16_t *eco2) {
    if (dev == NULL || eco2 == NULL)
        return ENS160_ERROR;
    *eco2 = dev->eco2;
    return ENS160_OK;
}

uint8_t ENS160_GetStatus(ENS160_t *dev, uint8_t *status) {
    if (dev == NULL || status == NULL)
        return ENS160_ERROR;
    *status = dev->status;
    return ENS160_OK;
}
