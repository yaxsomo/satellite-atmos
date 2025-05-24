#include "manager_h/manager.h"
#include "drivers_h/ms5607.h"
#include "drivers_h/sds011.h"
#include "drivers_h/ens160.h"
#include "drivers_h/aht21.h"
#include "tools_h/global_variables.h"
#include "drivers_h/led.h"
#include "drivers_h/black_box.h"
#include "tools_h/logger.h"
#include "fatfs.h"
#include "tools_h/configuration.h"

// EXTERN VARIABLES //
extern SystemState system_state;
extern Barometer_2_Axis barometer_data;
extern bool TAKEOFF_DETECTED;
extern bool TAKEOFF_ALREADY_DETECTED;
extern double ALTITUDE_MAX_GLOBAL;
extern float LAST_STORED_ALTITUDE_MAX;


SDS sds011_device;
ENS160_t ens160_device;

void get_timestamp(uint8_t *hour, uint8_t *min, uint8_t *sec, uint16_t *ms);
PhaseResult post_flight_phase(void);

void _Error_Handler(char *file, int line)
{
	while(1) {}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    sds_uart_RxCpltCallback(&sds011_device, &huart3);
}


// --- PHASE: INIT ---
PhaseResult init_phase(){
    LED_SetState(STATUS_INITIALIZATION);

    // SD card & CSV creation
    if(mount_sd_card() != 0){
        LED_SetState(STATUS_ERROR);
        log_event(0,0,0,0,"ERROR","SD card mount failed");
        return PHASE_FAIL;
    }
    black_box_init();
    check_free_space();
    log_event(0,0,0,0,"INFO","SD card mounted and log/telemetry ready");

    // Sensors init
    if(MS5607_Init() != 0){
        LED_SetState(STATUS_ERROR);
        log_event(0,0,0,0,"ERROR","MS5607 initialization failed");
        return PHASE_FAIL;
    }
    log_event(0,0,0,0,"INFO","MS5607 initialized");

    if(sdsInit(&sds011_device, &huart3) != 0){
        LED_SetState(STATUS_ERROR);
        log_event(0,0,0,0,"ERROR","SDS011 initialization failed");
        return PHASE_FAIL;
    }
    log_event(0,0,0,0,"INFO","SDS011 initialized");

    ENS160_Init(&ens160_device); // No return value, assumed always successful for now
    ENS160_SetMode(&ens160_device, ENS160_OPMODE_STD);
    log_event(0,0,0,0,"INFO","ENS160 initialized");

    if(AHT21_init() != 0){
        LED_SetState(STATUS_ERROR);
        log_event(0,0,0,0,"ERROR","AHT21 initialization failed");
        return PHASE_FAIL;
    }
    log_event(0,0,0,0,"INFO","AHT21 initialized");

    system_state = STATUS_PREFLIGHT;
    log_event(0,0,0,0,"INFO","System init complete. Ready for pre-flight.");
    return PHASE_SUCCESS;
}

// --- PHASE: PRE-FLIGHT ---
PhaseResult pre_flight_phase() {
    LED_SetState(STATUS_PREFLIGHT);
    log_print("[STATE] Waiting for Takeoff Detection...\n");
    log_event(0, 0, 0, 0, "STATE", "Waiting for Takeoff Detection...");

    while (system_state == STATUS_PREFLIGHT) {
        barometer_data = MS5607_ReadData();

        log_print("[BAROMETER] Pressure: %.3f Pa, Temp: %.3f degC, Altitude: %.3f meters\n",
                  barometer_data.pressure, barometer_data.temperature, barometer_data.altitude);

        // Event log to SD as well
        uint8_t hour, min, sec; uint16_t ms;
        get_timestamp(&hour, &min, &sec, &ms);
        char msg[128];
        snprintf(msg, sizeof(msg), "[BAROMETER] Pressure: %.3f Pa, Temp: %.3f degC, Altitude: %.3f meters",
                 barometer_data.pressure, barometer_data.temperature, barometer_data.altitude);
        log_event(hour, min, sec, ms, "BAROMETER", msg);

        if (barometer_data.altitude > ALTITUDE_MAX_GLOBAL) {
            ALTITUDE_MAX_GLOBAL = barometer_data.altitude;
        }

        if (!TAKEOFF_ALREADY_DETECTED && barometer_data.altitude > TAKEOFF_ALTITUDE_THRESHOLD) {
            TAKEOFF_DETECTED = true;
            TAKEOFF_ALREADY_DETECTED = true;
            log_print("[STATE] TAKEOFF DETECTED!\n");
            log_event(hour, min, sec, ms, "STATE", "TAKEOFF DETECTED!");
        }

        if (TAKEOFF_DETECTED) {
            log_print("[STATE] Transition to Flight Mode\n");
            log_event(hour, min, sec, ms, "STATE", "Transition to Flight Mode");
            system_state = STATUS_FLIGHT;
            return PHASE_SUCCESS;
        }
    }
    log_print("[STATE] Interrupted - Exiting Pre-Flight\n");
    log_event(0, 0, 0, 0, "STATE", "Interrupted - Exiting Pre-Flight");
    return PHASE_INTERRUPTED;
}

// --- PHASE: FLIGHT ---
PhaseResult flight_phase() {
    log_event(0,0,0,0,"STATE","Entered FLIGHT PHASE");
    LED_SetState(STATUS_FLIGHT);

    const int APOGEE_DEBOUNCE = 5;
    int apogee_measures = APOGEE_DEBOUNCE;
    float last_altitude = 0.0f;
    bool apogee_detected = false;
    bool touchdown_detected = false;

    // (Optional) Prepare any deploy logic/flags

    while (system_state == STATUS_FLIGHT) {
        // --- Timestamp
        uint8_t hour, min, sec;
        uint16_t ms;
        get_timestamp(&hour, &min, &sec, &ms);

        // --- 1. Barometer
        barometer_data = MS5607_ReadData();
        float current_altitude = barometer_data.altitude;

        // --- 2. SDS011 readings
        float pm2_5 = (float)sdsGetPm2_5(&sds011_device);
        float pm10  = (float)sdsGetPm10(&sds011_device);

        // --- 3. ENS160 readings
        ENS160_ReadData(&ens160_device);
        uint8_t AQI         = ens160_device.aqi;
        uint16_t TVOC       = ens160_device.tvoc;
        uint16_t eCO2       = ens160_device.eco2;

        // --- 4. AHT21 readings
        float aht21_temperature = (float)AHT21_Read_Temperature();
        float aht21_humidity    = (float)AHT21_Read_Humidity();

        // --- 5. Telemetry log
        log_telemetry(hour, min, sec, ms,
                      barometer_data.temperature,
                      barometer_data.pressure,
                      barometer_data.altitude,
                      pm2_5,
                      pm10,
                      AQI,
                      TVOC,
                      eCO2,
                      aht21_temperature,
                      aht21_humidity);

        // --- 6. Apogee detection (debounce style)
        if (current_altitude > ALTITUDE_MAX_GLOBAL) {
            ALTITUDE_MAX_GLOBAL = current_altitude;
        }

        if (!apogee_detected) {
            if (current_altitude < last_altitude) {
                apogee_measures--;
                if (apogee_measures <= 0) {
                    apogee_detected = true;
                    char msg[64];
                    snprintf(msg, sizeof(msg), "APOGEE detected at %.2f meters!", ALTITUDE_MAX_GLOBAL);
                    log_event(hour, min, sec, ms, "EVENT", msg);
                }
            } else {
                apogee_measures = APOGEE_DEBOUNCE;
            }
            last_altitude = current_altitude;
        } else {
            // --- 7. Touchdown detection (after apogee)
            if (!touchdown_detected && (current_altitude < (TOUCHDOWN_ALTITUDE_THRESHOLD + 0.5))) {
                touchdown_detected = true;
                char msg[64];
                snprintf(msg, sizeof(msg), "TOUCHDOWN detected at %.2f m", current_altitude);
                log_event(hour, min, sec, ms, "EVENT", msg);
                system_state = STATUS_POSTFLIGHT;
                break;
            }
        }

        //HAL_Delay(FLIGHT_LOG_DELAY_MS);
    }

    return PHASE_SUCCESS;
}

// --- PHASE: POST-FLIGHT ---
PhaseResult post_flight_phase() {
    log_event(0,0,0,0,"STATE","Entered POST-FLIGHT PHASE");
    LED_SetState(STATUS_GRACEFUL_SHUTDOWN);

    // Flush files & unmount
    black_box_flush_all();
    log_event(0,0,0,0,"INFO","SD files flushed and closed.");
    unmount_sd_card();
    //log_event(0,0,0,0,"INFO","SD card unmounted.");

    return PHASE_SUCCESS;
}

// --- MAIN MANAGER LOGIC ---
SystemState Manager_Main() {
    log_event(0,0,0,0,"INFO","Sat Atmo - Diamant A Experience - Welcome!");

    PhaseResult ret = init_phase();
    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    ret = pre_flight_phase();
    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    ret = flight_phase();
    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    ret = post_flight_phase();
    if(ret != PHASE_SUCCESS) return STATUS_ERROR;

    return STATUS_GRACEFUL_SHUTDOWN;
}

// --- Timestamp function (stub) ---
void get_timestamp(uint8_t *hour, uint8_t *min, uint8_t *sec, uint16_t *ms) {
    // TODO: Replace with your actual RTC/timebase
    static uint32_t counter = 0;
    *hour = 0;
    *min = 0;
    *sec = (counter / 20) % 60;
    *ms = (counter % 20) * 50;
    counter++;
}
