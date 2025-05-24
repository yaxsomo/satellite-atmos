/*
 * mics5524_ll.h
 *
 *  Created on: May 21, 2025
 *      Author: Yassine DEHHANI
 */

#ifndef INC_DRIVERS_H_MICS5524_LL_H_
#define INC_DRIVERS_H_MICS5524_LL_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define MICS5524_OX_MODE      0x00
#define MICS5524_RED_MODE     0x01

#define MICS5524_CO           0x01
#define MICS5524_CH4          0x02
#define MICS5524_C2H5OH       0x03
#define MICS5524_H2           0x06
#define MICS5524_NH3          0x08
#define MICS5524_NO2          0x0A

#define MICS5524_EXIST        0x00
#define MICS5524_NO_EXIST     0x02
#define MICS5524_ERROR       -1

#define ADC_MAX              4095

typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t adc_channel;  // e.g. ADC_CHANNEL_3
    uint16_t r0_ox;
    uint16_t r0_red;
    uint32_t warmup_time_ms;
    uint32_t start_time_ms;
    uint8_t  flag;
} MICS5524_t;

void     MICS5524_Init(MICS5524_t* sensor, ADC_HandleTypeDef* hadc, uint32_t adc_channel);
uint8_t  MICS5524_WarmUp(MICS5524_t* sensor, uint8_t minutes);
int16_t  MICS5524_ReadADC(MICS5524_t* sensor);
float    MICS5524_GetGasData(MICS5524_t* sensor, uint8_t type);
int8_t   MICS5524_GetGasExist(MICS5524_t* sensor, uint8_t gas);
float get_co_ppm(float rs_r0);
float MICS5524_CalculateRs(uint16_t adc_val, float rl_value, float vref);
// Optionally: helper for millis (must implement yourself)
extern uint32_t MICS5524_GetMillis(void);

#endif /* INC_DRIVERS_H_MICS5524_LL_H_ */
