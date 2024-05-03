#ifndef FIRMWARE__H
#define FIRMWARE__H

// System Includes
#include <esp_http_client.h>
#include <esp_ota_ops.h>
#include "esp_log.h"
#include "nvs_flash.h"

// Project includes
#include "main.h"


// Functions
#define TAG_FIRMWARE        "firmware"
#define NVS_PARTITION_NAME  "nvs"
#define NAMESPACE_STORAGE   "storage"


void firmwareUpdate(void);
void startBootloader(void);

esp_err_t nvsInit();

esp_err_t configurationRead(const char *variable, char *value, size_t *length);
esp_err_t configurationWrite(char *variable, char *value);
void      configurationSetup(void);

bool StrToUnsignedShort8(const char *text, uint8_t *value);

#endif