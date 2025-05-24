/*
 * configuration.h
 *
 *  Created on: Feb 5, 2025
 *      Author: yaxsomo
 */

#ifndef INC_CONFIGURATION_H_
#define INC_CONFIGURATION_H_

#include "stm32l4xx_hal.h"


/* ========================== */
/*       EXTERN INTERFACES    */
/* ========================== */

//extern TIM_HandleTypeDef htimX; // Timer for operations

extern SPI_HandleTypeDef hspi1; // BAROMETER SPI INTERFACE
extern ADC_HandleTypeDef hadc1;

/* ========================== */
/*        ENUM DEFINITIONS    */
/* ========================== */

typedef enum {
    DISABLED = 0,
    ENABLED = 1
} PropertyState;



/* ========================== */
/*        GENERAL SETTINGS     */
/* ========================== */

#define FIRMWARE_VERSION       "Satellite_Atmos v1.0"


/*         DEBUG SETTINGS     */
/* ========================== */

#define LOGGER                   // Debugging mode
#define SWV_DEBUG                // SWV Debugging
//#define STORAGE_LOGS             // Store Logs on the external SD Card
//#define STORAGE_TELEMETRY        // Store Telemetry on the external SD Card
#define LOG_BUFFER_SIZE        256




/* ========================== */
/*        FLIGHT CHECKS       */
/* ========================== */


#define WAIT_STATE_GLOBAL_DELAY                   500      // Delay between operations on the wait state
#define WAIT_STATE_DELAY_BETWEEN_OPERATIONS       1000      // Delay between operations on the wait state

/* ========================== */
/*       STUB FUNCTION        */
/* ========================== */

#define STUB_FUNCTION(func_name) \
    printf("[WARNING] Function %s is not implemented yet.\n", func_name);

/* ========================== */
/*        PIN DEFINITIONS     */
/* ========================== */


/* Individual LEDs */
#define LED_OK_Pin GPIO_PIN_7
#define LED_OK_GPIO_Port GPIOC
#define LED_ERROR_Pin GPIO_PIN_8
#define LED_ERROR_GPIO_Port GPIOC


#define LED_LONG_ON_VALUE 500
#define LED_LONG_OFF_VALUE 500

#define LED_SHORT_ON_VALUE 200
#define LED_SHORT_OFF_VALUE 200


/* Barometer */

#define BARO_CS_Pin GPIO_PIN_4
#define BARO_CS_GPIO_Port GPIOA

#define TAKEOFF_ALTITUDE_THRESHOLD 20.0f
#define APOGEE_COUNT_THRESHOLD     5

#define NUM_SAMPLES 100  // Number of readings to average
#define P0  1013.25     // Pressure at sea level
#define SEA_LEVEL_PRESSURE 102450.0  // Sea level standard atmospheric pressure in Pa
#define GAS_CONSTANT 8.31432         // Universal gas constant in N·m/(mol·K)
#define GRAVITY 9.80665              // Acceleration due to gravity in m/s²
#define MOLAR_MASS_AIR 0.0289644         // Molar mass of Earth's air in kg/mol
#define TEMP_LAPSE_RATE 0.0065       // Temperature lapse rate in K/m
#define STANDARD_TEMP 288.15         // Standard temperature at sea level in K
#define PASCAL_TO_HECTOPASCAL 100 	 //Divide the pressure by this number to get hPa
#define PASCAL_TO_KILOPASCAL 1000	 //Divide the pressure by this number to get kPa



/* MICS5524 Gas Sensor */

// === MICS5524 Sensor Configuration ===
#define MICS5524_VREF         5.0f  // Tension de référence ADC (en V)
#define MICS5524_ADC_HANDLE   hadc1 // ADC utilisé (défini dans ton main.c)


/* SD CARD */

#define SPI_TIMEOUT 100

extern SPI_HandleTypeDef 	hspi2;
#define HSPI_SDCARD		 	&hspi2
#define	SD_CS_PORT			GPIOB
#define SD_CS_PIN			GPIO_PIN_12

#endif /* INC_CONFIGURATION_H_ */
