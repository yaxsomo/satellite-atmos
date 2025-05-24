/*
 * manager.c
 *
 *  Created on: May 21, 2025
 *      Author: Yassine DEHHANI
 */
/* --- MODIFIED state_manager.c --- */
#include "manager_h/manager.h"
#include "drivers_h/ms5607.h"
#include "drivers_h/mics5524.h"
#include "tools_h/global_variables.h"
#include "drivers_h/led.h"
#include "tools_h/logger.h"
#include "drivers_h/fatfs_sd.h"
#include "fatfs.h"

// EXTERN VARIABLES //
extern SystemState system_state;
extern Barometer_2_Axis barometer_data;
extern bool TAKEOFF_DETECTED;
extern bool TAKEOFF_ALREADY_DETECTED;
extern double ALTITUDE_MAX_GLOBAL;
extern float LAST_STORED_ALTITUDE_MAX;

MICS5524_Handle_t gas_sensor;
float voltage = 0;


void _Error_Handler(char *file, int line)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

void test_sd_card() {
	const char total_uptime_filename[] = "uptime.dat";
	const char tick_filename[] = "tick.txt";
	const char big_filename[] = "big.dat";
	uint32_t total_uptime = 0;
	 uint32_t wbytes, rbytes; /* File write counts */

	   if (f_mount(&USERFatFS, "", 0) != FR_OK) {
	        printf("Unable to mount disk\n");
	        Error_Handler();
	    }

	    if (f_open(&USERFile, total_uptime_filename, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
	        if (f_read(&USERFile, &total_uptime, sizeof(total_uptime), (void*) &rbytes) == FR_OK) {
	            printf("Total uptime = %lu\n", total_uptime);
	            f_close(&USERFile);
	        } else {
	            printf("Unable to read\n");
	            Error_Handler();
	        }
	    } else {
	        // File did not exist - let's create it
	        if (f_open(&USERFile, total_uptime_filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
	            if (f_write(&USERFile, &total_uptime, sizeof(total_uptime), (void*) &wbytes) == FR_OK) {
	                printf("File %s created\n", total_uptime_filename);
	                f_close(&USERFile);
	            } else {
	                printf("Unable to write\n");
	                Error_Handler();
	            }
	        } else {
	            printf("Unable to create\n");
	            Error_Handler();
	        }
	    }

	    // Create tick file if it does NOT exist
	    if (f_open(&USERFile, tick_filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
	        f_close(&USERFile);
	    }

	    uint8_t buf[1024]; // 1K buffer
	    for (uint16_t i = 0; i < 1024; ++i) {
	        buf[i] = (uint8_t) i;
	    }

	    uint32_t start = uwTick;
	    if (f_open(&USERFile, big_filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
	        for (uint16_t i = 0; i < 100; ++i) {
	            if (f_write(&USERFile, &buf, sizeof(buf), (void*) &wbytes) != FR_OK) {
	                printf("Unable to write\n");
	            }
	        }
	        f_close(&USERFile);
	    } else {
	        printf("Unable to open %s\n", big_filename);
	    }
	    printf("Write took %lu ms\n", uwTick - start);
}



PhaseResult init_phase(){
    LED_SetState(STATUS_INITIALIZATION);

    int8_t barometer_init_result = MS5607_Init();
    if(barometer_init_result != 0){
        LED_SetState(STATUS_ERROR);
        return PHASE_FAIL;
    }

    MICS5524_Handle_Init(&gas_sensor, &hadc1, ADC_CHANNEL_3);

    log_print("[GAS SENSOR] Warming up...\n");
    // Block here (startup only) until warmup done
    while (!MICS5524_Handle_WarmUp(&gas_sensor, 0.3)) {
        HAL_Delay(100); // or your preferred delay
    }
    log_print("[GAS SENSOR] Warmup complete!\n");


    system_state = STATUS_PREFLIGHT;
    return PHASE_SUCCESS;
}



PhaseResult pre_flight_phase() {
    LED_SetState(STATUS_PREFLIGHT);
    log_print("[STATE] Waiting for Takeoff Detection...\n");

    while (system_state == STATUS_PREFLIGHT) {
        barometer_data = MS5607_ReadData();

        log_print("[BAROMETER] Pressure: %.3f Pa, Temp: %.3f degC, Altitude: %.3f meters\n",
                  barometer_data.pressure, barometer_data.temperature, barometer_data.altitude);

        if (barometer_data.altitude > ALTITUDE_MAX_GLOBAL) {
            ALTITUDE_MAX_GLOBAL = barometer_data.altitude;
        }

        if (!TAKEOFF_ALREADY_DETECTED && barometer_data.altitude > TAKEOFF_ALTITUDE_THRESHOLD) {
            TAKEOFF_DETECTED = true;
            TAKEOFF_ALREADY_DETECTED = true;
            log_print("[STATE] TAKEOFF DETECTED!\n");
        }

        if (TAKEOFF_DETECTED) {
            log_print("[STATE] Transition to Flight Mode\n");
            system_state = STATUS_FLIGHT;
            return PHASE_SUCCESS;
        }
    }
    log_print("[STATE] Interrupted - Exiting Pre-Flight\n");
    return PHASE_INTERRUPTED;
}


PhaseResult flight_phase() {

	while(true){
//	    // Sensor already warmed up!
//	    float co_ppm      = MICS5524_Handle_ReadGasPPM(&gas_sensor, MICS5524_CO);
//	    float ch4_ppm     = MICS5524_Handle_ReadGasPPM(&gas_sensor, MICS5524_CH4);
//	    float ethanol_ppm = MICS5524_Handle_ReadGasPPM(&gas_sensor, MICS5524_C2H5OH);
//	    float h2_ppm      = MICS5524_Handle_ReadGasPPM(&gas_sensor, MICS5524_H2);
//	    float nh3_ppm     = MICS5524_Handle_ReadGasPPM(&gas_sensor, MICS5524_NH3);
//
//	    int co_present    = MICS5524_Handle_GasPresent(&gas_sensor, MICS5524_CO);
//	    int ch4_present   = MICS5524_Handle_GasPresent(&gas_sensor, MICS5524_CH4);
//	    int etoh_present  = MICS5524_Handle_GasPresent(&gas_sensor, MICS5524_C2H5OH);
//	    int h2_present    = MICS5524_Handle_GasPresent(&gas_sensor, MICS5524_H2);
//	    int nh3_present   = MICS5524_Handle_GasPresent(&gas_sensor, MICS5524_NH3);
//
//	    log_print("[GAS] CO: %.2fppm (%s), CH4: %.2fppm (%s), EtOH: %.2fppm (%s), H2: %.2fppm (%s), NH3: %.2fppm (%s)\n",
//	        co_ppm,   co_present   ? "YES":"NO",
//	        ch4_ppm,  ch4_present  ? "YES":"NO",
//	        ethanol_ppm, etoh_present ? "YES":"NO",
//	        h2_ppm,   h2_present   ? "YES":"NO",
//	        nh3_ppm,  nh3_present  ? "YES":"NO"
//	    );
#define RL_VALUE 10000.0f // Valeur de la résistance de charge en ohms
#define VREF 3.3f          // Tension de référence de l'ADC

float rs = MICS5524_CalculateRs(adc_val, RL_VALUE, VREF);
float rs_r0 = rs / r0_ox;
float co_ppm = get_co_ppm(rs_r0);
//		int16_t adc_val = MICS5524_ReadADC(&gas_sensor.ll_sensor);
//		log_print("valeur: %d", adc_val);
//		float test_var = MICS5524_ReadVoltage();

	    HAL_Delay(1000);
	}



    return PHASE_SUCCESS;
}


SystemState Manager_Main(){
	 log_print("Sat Atmo - Diamant A Experience - Welcome! \n");
	 //test_sd_card();

    PhaseResult ret = init_phase();
    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

	ret = pre_flight_phase();
	if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    ret = flight_phase();
	if(ret != PHASE_SUCCESS) return STATUS_ERROR;
//
//    ret = post_flight_phase();
//    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    LED_SetState(STATUS_GRACEFUL_SHUTDOWN);
    return STATUS_GRACEFUL_SHUTDOWN;
}


