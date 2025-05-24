/*
 * mics5524.h
 *
 *  Created on: May 21, 2025
 *      Author: Yassine DEHHANI
 */

#ifndef INC_DRIVERS_H_MICS5524_H_
#define INC_DRIVERS_H_MICS5524_H_

#include "mics5524_ll.h"

typedef struct {
    MICS5524_t ll_sensor; // Low-level driver struct
    // Optionally add any app-level members here (e.g., last values, status, etc.)
} MICS5524_Handle_t;

// High-level functions
void MICS5524_Handle_Init(MICS5524_Handle_t* handle, ADC_HandleTypeDef* hadc, uint32_t adc_channel);
uint8_t MICS5524_Handle_WarmUp(MICS5524_Handle_t* handle, uint8_t minutes);
float   MICS5524_Handle_ReadGasPPM(MICS5524_Handle_t* handle, uint8_t gas_type);
int8_t  MICS5524_Handle_GasPresent(MICS5524_Handle_t* handle, uint8_t gas_type);
uint16_t MICS5524_ReadRaw(void);
float MICS5524_ReadVoltage(void);
#endif /* INC_DRIVERS_H_MICS5524_H_ */
