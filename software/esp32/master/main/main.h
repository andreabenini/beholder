#ifndef MAIN__H
#define MAIN__H

#include "esp_task_wdt.h"

/**
 * Set project log level, Default is ESP_LOG_INFO. This define can disable logging all at once (for dealing with other serial devices for example)
 * Log level can also be changed with:
 *      - CONFIG_LOG_MAXIMUM_LEVEL setting in menuconfig
 *      - esp_log_level_set() in esp_log.h
 * Possible log values are:
 *      ESP_LOG_NONE
 *      ESP_LOG_ERROR
 *      ESP_LOG_WARN
 *      ESP_LOG_INFO        (default)
 *      ESP_LOG_DEBUG
 *      ESP_LOG_VERBOSE
 */
// #define LOG_LOCAL_LEVEL ESP_LOG_NONE


// Program configuration variables
#define VAR_LEN                     100

#define VAR_WIFI_SSID               "wifissid"
#define VAR_WIFI_PASSWORD           "wifipassword"
#define VAR_UPDATE_URL              "firmwareurl"
#define VAR_SECURITY_KEY            "secret"
// Bluetooth variables
#define VAR_BT_ADDRESS              "bluetooth"
// Camera variables
#define VAR_CAMERA_RESOLUTION       "cam_resolution"
#define VAR_CAMERA_QUALITY          "cam_quality"

#include "secret.h"
// Inside secret.h there are just these two definitions, feel free to create
// your secret.h file and keep them separated or uncomment those two lines below
// #define WIFI_SSID                   "yourSSID"
// #define WIFI_PASSWORD               "SecretWIFIPassword"
// [Default values] Wireless configuration
#define WIFI_MAX_RETRIES            5
#define WIFI_RETRY_TIMEOUT          1500        // ms
// [Default values] Bluetooth configuration
#define DEFAULT_BT_MAC_ADDRESS      "1a:2b:3c:01:01:01"
// [Default values] Firmware updates
#define DEFAULT_FIRMWARE_URL        "http://192.168.0.16:8000/firmware"
#define DEFAULT_SECURITY_KEY        "42"
// [Default values] camera settings
#define DEFAULT_CAMERA_RESOLUTION   "8"         // (sensor.h) typedef enum framesize_t
#define DEFAULT_CAMERA_QUALITY      "30"


// Camera configuration
// Program configuration
#define TAG_APP                     "Rollie"
// #define BOOTLOADER_ONLY


// User defined functions
// #define PROGRAM_HALT                esp_task_wdt_delete(NULL); while (1){vTaskDelay(portMAX_DELAY);}
#define PROGRAM_HALT                while (1){vTaskDelay(portMAX_DELAY);}

#endif
