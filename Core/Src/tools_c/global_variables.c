/*
 * global_variables.c
 *
 *  Created on: Feb 5, 2025
 *      Author: yaxsomo
 */

#include "tools_h/global_variables.h"
#include "tools_h/configuration.h"
#include "drivers_h/ms5607.h"


// State machine initialization
SystemState system_state = STATUS_INITIALIZATION;

Barometer_2_Axis barometer_data;

bool TAKEOFF_DETECTED = false;
bool TAKEOFF_ALREADY_DETECTED = false;

float LAST_STORED_ALTITUDE_MAX = 0;
float ALTITUDE_MAX_GLOBAL = 0;

