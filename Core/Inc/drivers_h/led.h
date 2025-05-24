/*
 * led.h
 *
 *  Created on: Feb 5, 2025
 *      Author: yaxsomo
 */

#ifndef INC_DRIVERS_H_LED_LED_H_
#define INC_DRIVERS_H_LED_LED_H_

#include "tools_h/global_variables.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

/* ========================== */
/*        LED DEFINITIONS     */
/* ========================== */

typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_SOLID,
    LED_MODE_BLINK,
} LED_Mode;

typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
    LED_Mode mode;
    uint32_t last_toggle_time;
    GPIO_PinState current_state;
} IndividualLED;

/* ========================== */
/*        FUNCTION PROTOTYPES */
/* ========================== */

void LED_Init(void);
void LED_Update(void);
void LED_SetOK(LED_Mode mode);
void LED_SetError(LED_Mode mode);
void LED_SetState(SystemState state);

#endif /* INC_DRIVERS_H_LED_LED_H_ */
