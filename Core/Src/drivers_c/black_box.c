/*
 * BlackBox.c
 *
 *  Created on: Jul 20, 2023
 *      Author: yaxsomo
 */

#include <drivers_h/black_box.h>
#include "fatfs.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "tools_h/configuration.h"

// SD Card objects
FATFS fs;
FRESULT fresult;
BYTE work[4096];



// Logging file handles and counters
static FIL log_fil;
static FIL telemetry_fil;
static bool log_file_ready = false;
static bool telemetry_file_ready = false;
static uint32_t log_write_counter = 0;
static uint32_t telemetry_write_counter = 0;
static char log_filename[32] = "logs.csv";
static char telemetry_filename[32] = "telemetry.csv";

// FatFS result strings for debug
const char* fresultStrings[] = {
    "FR_OK","FR_DISK_ERR","FR_INT_ERR","FR_NOT_READY","FR_NO_FILE","FR_NO_PATH",
    "FR_INVALID_NAME","FR_DENIED","FR_EXIST","FR_INVALID_OBJECT","FR_WRITE_PROTECTED",
    "FR_INVALID_DRIVE","FR_NOT_ENABLED","FR_NO_FILESYSTEM","FR_MKFS_ABORTED","FR_TIMEOUT",
    "FR_LOCKED","FR_NOT_ENOUGH_CORE","FR_TOO_MANY_OPEN_FILES","FR_INVALID_PARAMETER"
};

static void get_next_available_filename(const char *base, const char *ext, char *out, int outlen) {
    int idx = 1;
    do {
        snprintf(out, outlen, "%s%03d.%s", base, idx, ext);
        if (!file_exists(out)) {
            break;
        }
        idx++;
    } while (idx < 1000);
}

bool file_exists(const char* filename) {
    FILINFO fno;
    FRESULT result = f_stat(filename, &fno);
    return result == FR_OK && !(fno.fattrib & AM_DIR);
}

// Mount SD card (returns 0 if OK, 1 if error)
int8_t mount_sd_card(){
    fresult = f_mount(&fs, "0:/",1);
    if(fresult != FR_OK) {
        printf("*               Error mounting the SD Card              *\r\n");
        return 1;
    } else {
        printf("*              SD Card mounted successfully!            *\r\n");
        return 0;
    }
}

// Unmount SD card
void unmount_sd_card(){
    fresult = f_mount(NULL, "/", 1);
    if (fresult == FR_OK) printf ("SD CARD UNMOUNTED successfully...\r\n");
}

// Print free space in GB
void check_free_space(void) {
    FATFS* pfs;
    DWORD fre_clust;
    FRESULT fres;
    DWORD free_sectors, total_sectors;

    fres = f_getfree("", &fre_clust, &pfs);
    if (fres != FR_OK) {
        printf("f_getfree error (%i)\r\n", fres);
        while(1);
    }

    total_sectors = (pfs->n_fatent - 2) * pfs->csize;
    free_sectors = fre_clust * pfs->csize;

    float total_GB = (float)total_sectors * 512.0f / (1024.0f * 1024.0f * 1024.0f);
    float free_GB  = (float)free_sectors * 512.0f / (1024.0f * 1024.0f * 1024.0f);

    printf("SD card stats:\r\n%10.2f GB total drive space.\r\n%10.2f GB available.\r\n",
           total_GB, free_GB);
}

// --- Internal: Write file header if missing, open for append ---
static FRESULT ensure_file_with_header(const char* filename, const char* header, FIL* f) {
    if (!file_exists(filename)) {
        FRESULT res = f_open(f, filename, FA_CREATE_ALWAYS | FA_WRITE);
        if (res == FR_OK) {
            UINT bw;
            f_write(f, header, strlen(header), &bw);
            f_sync(f);
            f_close(f);
        } else {
            return res;
        }
    }
    // Open for appending
    return f_open(f, filename, FA_OPEN_APPEND | FA_WRITE);
}

// --- Init and flush functions ---
void black_box_init(void) {
    log_file_ready = false;
    telemetry_file_ready = false;
    log_write_counter = 0;
    telemetry_write_counter = 0;
    // Generate unique filenames for this session
    get_next_available_filename("logs", "csv", log_filename, sizeof(log_filename));
    get_next_available_filename("telemetry", "csv", telemetry_filename, sizeof(telemetry_filename));
}


void black_box_flush_all(void) {
    if (log_file_ready) {
        f_sync(&log_fil);
        f_close(&log_fil);
        log_file_ready = false;
    }
    if (telemetry_file_ready) {
        f_sync(&telemetry_fil);
        f_close(&telemetry_fil);
        telemetry_file_ready = false;
    }
}

// --- Event Logging: logs.csv ---
void log_event(uint8_t hour, uint8_t min, uint8_t sec, uint16_t ms,
               const char* log_level, const char* message) {
    FRESULT res;
    char line[256];

    if (!log_file_ready) {
        const char* header = "TIMESTAMP,LOG_LEVEL,MESSAGE\r\n";
        res = ensure_file_with_header(log_filename, header, &log_fil);
        if (res != FR_OK) {
            printf("Can't open log file! FR = %d\r\n", res);
            return;
        }
        log_file_ready = true;
        log_write_counter = 0;
    }

    snprintf(line, sizeof(line), "%02u:%02u:%02u:%03u,%s,%s\r\n",
             hour, min, sec, ms, log_level, message);

    UINT bw;
    res = f_write(&log_fil, line, strlen(line), &bw);
    if (res != FR_OK || bw != strlen(line)) {
        printf("Log file write failed! FR = %d, bytes = %u\r\n", res, bw);
        log_file_ready = false;
        f_close(&log_fil);
        return;
    }

    log_write_counter++;
    if (log_write_counter >= LOG_BURST_N) {
        f_sync(&log_fil);
        f_close(&log_fil);
        log_file_ready = false;
    }
}

// --- Telemetry Logging: telemetry.csv ---
void log_telemetry(uint8_t hour, uint8_t min, uint8_t sec, uint16_t ms,
                   float ms5607_temperature, float ms5607_pressure, float ms5607_altitude,
                   float sds011_pm2_5, float sds011_pm10,
                   float ens160_AQI, float ens160_TVOC, float ens160_eCO2,
                   float aht21_temperature, float aht21_humidity) {
    FRESULT res;
    char line[320];

    if (!telemetry_file_ready) {
        const char* header = "TIMESTAMP,ms5607_temperature,ms5607_pressure,ms5607_altitude,"
                             "sds011_pm2_5,sds011_pm10,ens160_AQI,ens160_TVOC,ens160_eCO2,"
                             "aht21_temperature,aht21_humidity\r\n";
        res = ensure_file_with_header(telemetry_filename, header, &telemetry_fil);
        if (res != FR_OK) {
            printf("Can't open telemetry file! FR = %d\r\n", res);
            return;
        }
        telemetry_file_ready = true;
        telemetry_write_counter = 0;
    }

    snprintf(line, sizeof(line),
             "%02u:%02u:%02u:%03u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
             hour, min, sec, ms,
             ms5607_temperature, ms5607_pressure, ms5607_altitude,
             sds011_pm2_5, sds011_pm10,
             ens160_AQI, ens160_TVOC, ens160_eCO2,
             aht21_temperature, aht21_humidity);

    UINT bw;
    res = f_write(&telemetry_fil, line, strlen(line), &bw);
    if (res != FR_OK || bw != strlen(line)) {
        printf("Telemetry file write failed! FR = %d, bytes = %u\r\n", res, bw);
        telemetry_file_ready = false;
        f_close(&telemetry_fil);
        return;
    }

    telemetry_write_counter++;
    if (telemetry_write_counter >= TELEMETRY_BURST_N) {
        f_sync(&telemetry_fil);
        f_close(&telemetry_fil);
        telemetry_file_ready = false;
    }
}
