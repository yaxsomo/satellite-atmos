/*
 * logger.h
 *
 *  Created on: Feb 5, 2025
 *      Author: yaxsomo
 */

#ifndef INC_TOOLS_H_LOGGER_H_
#define INC_TOOLS_H_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "stm32l4xx_hal.h"
#include "configuration.h"

/* ========================== */
/*        LOGGER FUNCTION     */
/* ========================== */

static inline void log_print(const char *format, ...) {
#ifdef LOGGER
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);

#ifdef SWV_DEBUG
    printf("%s", buffer); // SWV output
#endif

#endif // LOGGER
}


#endif /* INC_TOOLS_H_LOGGER_H_ */
