#ifndef MAIN__H
#define MAIN__H


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


// Camera configuration
// Program configuration
#define TAG_APP                     "BT Demo"
// #define BOOTLOADER_ONLY


#endif
