/*
 * mics5524_ll.c
 *
 *  Created on: May 21, 2025
 *      Author: Yassine DEHHANI
 */


#include "drivers_h/mics5524_ll.h"
#include <math.h>

static int16_t read_adc_channel(ADC_HandleTypeDef* hadc, uint32_t channel)
{
    HAL_ADC_Start(hadc);
    if(HAL_ADC_PollForConversion(hadc, 10) != HAL_OK) {
        return MICS5524_ERROR;
    }
    uint16_t value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);
    return value;
}


void MICS5524_Init(MICS5524_t* sensor, ADC_HandleTypeDef* hadc, uint32_t adc_channel)
{
    sensor->hadc = hadc;
    sensor->adc_channel = adc_channel;
    sensor->r0_ox = 0;
    sensor->r0_red = 0;
    sensor->flag = 0;
    sensor->start_time_ms = 0;
    sensor->warmup_time_ms = 0;
}

float MICS5524_CalculateRs(uint16_t adc_val, float rl_value, float vref)
{
    float voltage = ((float)adc_val / 4095.0f) * vref;
    return rl_value * (vref - voltage) / voltage;
}


uint8_t MICS5524_WarmUp(MICS5524_t* sensor, uint8_t minutes)
{
    if(sensor->flag == 0) {
        sensor->flag = 1;
        sensor->start_time_ms = MICS5524_GetMillis();
        sensor->warmup_time_ms = (uint32_t)minutes * 60000;
        return 0;
    }
    uint32_t elapsed = MICS5524_GetMillis() - sensor->start_time_ms;
    if(elapsed < sensor->warmup_time_ms) {
        return 0;
    }
    int16_t adc_val = read_adc_channel(sensor->hadc, sensor->adc_channel);
    if(adc_val == MICS5524_ERROR) return 0;
    // For analog sensors, only 1 channel is used, so powerData - ox/redData is not needed.
    // We set r0_ox and r0_red to baseline ADC value at end of warmup
    sensor->r0_ox = adc_val;
    sensor->r0_red = adc_val;
    return 1;
}

int16_t MICS5524_ReadADC(MICS5524_t* sensor)
{
    return read_adc_channel(sensor->hadc, sensor->adc_channel);
}

// --- Gas concentration math, ported from DFRobot Arduino library ---

static float get_co(float rs_r0)
{
    if(rs_r0 > 0.425) return 0.0f;
    float co = (0.425f - rs_r0) / 0.000405f;
    if(co > 1000.0f) return 1000.0f;
    if(co < 1.0f) return 0.0f;
    return co;
}

float get_co_ppm(float rs_r0)
{
    // Exemple basé sur une courbe logarithmique
    return pow(10, (log10(rs_r0) - b) / m);
}

static float get_ethanol(float rs_r0)
{
    if(rs_r0 > 0.306f) return 0.0f;
    float ethanol = (0.306f - rs_r0) / 0.00057f;
    if(ethanol < 10.0f) return 0.0f;
    if(ethanol > 500.0f) return 500.0f;
    return ethanol;
}

static float get_methane(float rs_r0)
{
    if(rs_r0 > 0.786f) return 0.0f;
    float methane = (0.786f - rs_r0) / 0.000023f;
    if(methane < 1000.0f) methane = 0.0f;
    if(methane > 25000.0f) methane = 25000.0f;
    return methane;
}

static float get_nitrogendioxide(float rs_r0)
{
    if(rs_r0 < 1.1f) return 0.0f;
    float nitrogendioxide = (rs_r0 - 0.045f) / 6.13f;
    if(nitrogendioxide < 0.1f) return 0.0f;
    if(nitrogendioxide > 10.0f) return 10.0f;
    return nitrogendioxide;
}

static float get_hydrogen(float rs_r0)
{
    if(rs_r0 > 0.279f) return 0.0f;
    float hydrogen = (0.279f - rs_r0) / 0.00026f;
    if(hydrogen < 1.0f) return 0.0f;
    if(hydrogen > 1000.0f) return 1000.0f;
    return hydrogen;
}

static float get_ammonia(float rs_r0)
{
    if(rs_r0 > 0.8f) return 0.0f;
    float ammonia = (0.8f - rs_r0) / 0.0015f;
    if(ammonia < 1.0f) return 0.0f;
    if(ammonia > 500.0f) return 500.0f;
    return ammonia;
}

// Returns "Rs/R0" (normalized ADC drop from baseline)
static float calc_rs_r0(MICS5524_t* sensor)
{
    int16_t adc_val = read_adc_channel(sensor->hadc, sensor->adc_channel);
    if(sensor->r0_ox == 0) return 0;
    return (float)(sensor->r0_ox - adc_val) / (float)sensor->r0_ox;
}

float MICS5524_GetGasData(MICS5524_t* sensor, uint8_t type)
{
    float rs_r0 = calc_rs_r0(sensor);
    switch(type) {
        case MICS5524_CO:     return get_co(rs_r0);
        case MICS5524_CH4:    return get_methane(rs_r0);
        case MICS5524_C2H5OH: return get_ethanol(rs_r0);
        case MICS5524_H2:     return get_hydrogen(rs_r0);
        case MICS5524_NH3:    return get_ammonia(rs_r0);
        case MICS5524_NO2:    return get_nitrogendioxide(rs_r0);
        default:              return MICS5524_ERROR;
    }
}

// --- Optional: "Exist" detection for binary gas presence ---

int8_t MICS5524_GetGasExist(MICS5524_t* sensor, uint8_t gas)
{
    float rs_r0 = calc_rs_r0(sensor);
    switch(gas) {
        case MICS5524_CO:     return rs_r0 <= 0.425f ? MICS5524_EXIST : MICS5524_NO_EXIST;
        case MICS5524_CH4:    return rs_r0 <= 0.786f ? MICS5524_EXIST : MICS5524_NO_EXIST;
        case MICS5524_C2H5OH: return rs_r0 <= 0.306f ? MICS5524_EXIST : MICS5524_NO_EXIST;
        case MICS5524_H2:     return rs_r0 <= 0.279f ? MICS5524_EXIST : MICS5524_NO_EXIST;
        case MICS5524_NH3:    return rs_r0 <= 0.8f   ? MICS5524_EXIST : MICS5524_NO_EXIST;
        case MICS5524_NO2:    return rs_r0 >= 1.1f   ? MICS5524_EXIST : MICS5524_NO_EXIST;
        default:              return MICS5524_ERROR;
    }
}

uint32_t MICS5524_GetMillis(void)
{
    return HAL_GetTick(); // STM32 HAL, returns milliseconds since startup
}
