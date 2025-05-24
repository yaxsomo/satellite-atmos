/*
 * mics5524.c
 *
 *  Created on: May 21, 2025
 *      Author: Yassine DEHHANI
 */


#include "drivers_h/mics5524.h"
#include "tools_h/configuration.h"
#include "stm32l4xx_hal.h"  // adapte à ta série STM32



// Initialize the high-level sensor handle (calls LL driver)
void MICS5524_Handle_Init(MICS5524_Handle_t* handle, ADC_HandleTypeDef* hadc, uint32_t adc_channel)
{
    MICS5524_Init(&handle->ll_sensor, hadc, adc_channel);
}

// Warm-up process (calls LL driver)
// Returns 1 if baseline is established, 0 otherwise
uint8_t MICS5524_Handle_WarmUp(MICS5524_Handle_t* handle, uint8_t minutes)
{
    return MICS5524_WarmUp(&handle->ll_sensor, minutes);
}

// Read a gas PPM value (calls LL driver)
// Returns: gas concentration in PPM (float), or MICS5524_ERROR if not supported
float MICS5524_Handle_ReadGasPPM(MICS5524_Handle_t* handle, uint8_t gas_type)
{
    return MICS5524_GetGasData(&handle->ll_sensor, gas_type);
}

// Binary gas detection (calls LL driver)
// Returns: MICS5524_EXIST, MICS5524_NO_EXIST, or MICS5524_ERROR
int8_t MICS5524_Handle_GasPresent(MICS5524_Handle_t* handle, uint8_t gas_type)
{
    return MICS5524_GetGasExist(&handle->ll_sensor, gas_type);
}


uint16_t MICS5524_ReadRaw(void) {
    HAL_ADC_Start(&MICS5524_ADC_HANDLE);
    HAL_ADC_PollForConversion(&MICS5524_ADC_HANDLE, 10);
    uint16_t raw = HAL_ADC_GetValue(&MICS5524_ADC_HANDLE);
    HAL_ADC_Stop(&MICS5524_ADC_HANDLE);
    return raw;
}

float MICS5524_ReadVoltage(void) {
    return ((float)MICS5524_ReadRaw() / 4095.0f) * MICS5524_VREF;
}
