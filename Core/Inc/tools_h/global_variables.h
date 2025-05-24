/*
 * global_variables.h
 *
 *  Created on: Feb 5, 2025
 *      Author: yaxsomo
 */

#ifndef INC_TOOLS_H_GLOBAL_VARIABLES_H_
#define INC_TOOLS_H_GLOBAL_VARIABLES_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    STATUS_GRACEFUL_SHUTDOWN = 0,
    STATUS_INITIALIZATION,
	STATUS_WAIT,
	STATUS_PREFLIGHT,
	STATUS_FLIGHT,
	STATUS_POSTFLIGHT,
	STATUS_SUCCESS,
	STATUS_ERROR,
} SystemState;
extern SystemState system_state;


typedef enum{
	PHASE_SUCCESS = 0,
	PHASE_INTERRUPTED,
	PHASE_FAIL
}PhaseResult;


#endif /* INC_TOOLS_H_GLOBAL_VARIABLES_H_ */
