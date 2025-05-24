#include "drivers_h/led.h"
#include "tools_h/configuration.h"
#include "tools_h/logger.h"
#include "tools_h/global_variables.h"

static IndividualLED led_ok = {
    .GPIO_Port = LED_OK_GPIO_Port,
    .GPIO_Pin = LED_OK_Pin,
    .mode = LED_MODE_OFF,
    .current_state = GPIO_PIN_RESET
};

static IndividualLED led_error = {
    .GPIO_Port = LED_ERROR_GPIO_Port,
    .GPIO_Pin = LED_ERROR_Pin,
    .mode = LED_MODE_OFF,
    .current_state = GPIO_PIN_RESET
};

void LED_Init(void) {
    HAL_GPIO_WritePin(led_ok.GPIO_Port, led_ok.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(led_error.GPIO_Port, led_error.GPIO_Pin, GPIO_PIN_RESET);
}

void LED_SetOK(LED_Mode mode) {
    led_ok.mode = mode;
    HAL_GPIO_WritePin(led_ok.GPIO_Port, led_ok.GPIO_Pin,
                      (mode == LED_MODE_SOLID) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    led_ok.current_state = (mode == LED_MODE_SOLID) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void LED_SetError(LED_Mode mode) {
    led_error.mode = mode;
    HAL_GPIO_WritePin(led_error.GPIO_Port, led_error.GPIO_Pin,
                      (mode == LED_MODE_SOLID) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    led_error.current_state = (mode == LED_MODE_SOLID) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void LED_Update(void) {
    // No blinking, nothing to update
}

void LED_SetState(SystemState state) {
    switch (state) {
        case STATUS_INITIALIZATION:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_PREFLIGHT:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_FLIGHT:
            LED_SetOK(LED_MODE_OFF);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_POSTFLIGHT:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_SUCCESS:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_ERROR:
            LED_SetOK(LED_MODE_OFF);
            LED_SetError(LED_MODE_SOLID);
            break;

        case STATUS_GRACEFUL_SHUTDOWN:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_OFF);
            break;

        case STATUS_WAIT:
            LED_SetOK(LED_MODE_SOLID);
            LED_SetError(LED_MODE_SOLID);
            break;

        default:
            LED_SetOK(LED_MODE_OFF);
            LED_SetError(LED_MODE_OFF);
            break;
    }

    log_print("[LED] State updated to %d\n", state);
}
