/*
 * BlackBox.h
 *
 *  Created on: Jul 20, 2023
 *      Author: yaxsomo
 */

#ifndef INC_BLACKBOX_H_
#define INC_BLACKBOX_H_

#include <stdbool.h>
#include <stdint.h>

// Basic SD functions
bool file_exists(const char* filename);
int8_t mount_sd_card(void);
void unmount_sd_card(void);
void check_free_space(void);

// Robust logger interface
void black_box_init(void);
void black_box_flush_all(void);

/**
 * Log an event.
 * Example: log_event(12,34,56,789,"INFO","System booted OK");
 */
void log_event(uint8_t hour, uint8_t min, uint8_t sec, uint16_t ms,
               const char* log_level, const char* message);

/**
 * Log telemetry data.
 * Example: log_telemetry(12,34,56,789,23.1,1012.2,56.7,8.5,11.0,2,415,620,22.0,38.0);
 */
void log_telemetry(uint8_t hour, uint8_t min, uint8_t sec, uint16_t ms,
                   float ms5607_temperature, float ms5607_pressure, float ms5607_altitude,
                   float sds011_pm2_5, float sds011_pm10,
                   float ens160_AQI, float ens160_TVOC, float ens160_eCO2,
                   float aht21_temperature, float aht21_humidity);

#endif /* INC_BLACKBOX_H_ */
