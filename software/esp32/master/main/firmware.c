#include "firmware.h"


/**
 * Callback HTTP client used to update the firmware
 * @see Use firmwareUpdate() from outside to execute this one
 */
esp_err_t firmwareUpdateCallback(esp_http_client_event_t *evt) {
    static esp_ota_handle_t ota_handle = 0;
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!ota_handle) {
                const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
                esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG_FIRMWARE, "OTA begin failed (%s)", esp_err_to_name(err));
                    return ESP_FAIL;
                }
                ESP_LOGI(TAG_FIRMWARE, "OTA begin succeeded");
            }
            if (esp_ota_write(ota_handle, evt->data, evt->data_len) != ESP_OK) {
                ESP_LOGE(TAG_FIRMWARE, "Error writing OTA data");
                return ESP_FAIL;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            if (esp_ota_end(ota_handle) != ESP_OK) {
                ESP_LOGE(TAG_FIRMWARE, "OTA end failed");
                return ESP_FAIL;
            }
            esp_err_t err = esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL));
            if (err != ESP_OK) {
                ESP_LOGE(TAG_FIRMWARE, "Set boot partition failed (%s)", esp_err_to_name(err));
                return ESP_FAIL;
            }
            ESP_LOGI(TAG_FIRMWARE, "OTA update successful, rebooting...");
            esp_restart();
            break;
        default:
            break;
    }
    return ESP_OK;
} /**/


/**
 * Firmware update function
 */
void firmwareUpdate(void) {
    char url[VAR_LEN];
    size_t urlLength = sizeof(url);
    if (configurationRead(VAR_UPDATE_URL, url, &urlLength) != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Cannot read update url from '%s' variable", VAR_UPDATE_URL);
        return;
    }
    ESP_LOGI(TAG_FIRMWARE, "Updating firmware from '%s'", url);
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = firmwareUpdateCallback,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "HTTP client perform failed (%s)", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
} /**/


/**
 * Basic bootloader on 'factory' partition, it just tells to reboot and use OTA_0 bigger partition for the whole stuff
 */
void startBootloader(void) {
    // Retrieve information about the partitions
    const esp_partition_t* firstPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    if (firstPartition == NULL) {
        ESP_LOGE(TAG_FIRMWARE, "Failed to find the first OTA partition\n");
        return;
    }
    // Set the boot partition to the first partition
    esp_err_t err = esp_ota_set_boot_partition(firstPartition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Failed to set boot partition OTA_0 (%s)\n", esp_err_to_name(err));
        return;
    }
    ESP_LOGW(TAG_FIRMWARE, "");
    ESP_LOGW(TAG_FIRMWARE, "Boot partition set to the first OTA partition");
    ESP_LOGW(TAG_FIRMWARE, "Restarting...");
    ESP_LOGW(TAG_FIRMWARE, "");
    esp_restart();
} /**/


/**
 * Read a configuration variable from the system flash
 */
esp_err_t configurationRead(const char *variable, char *value, size_t *length) {
    nvs_handle_t nvsHandle;
    esp_err_t ret = nvs_open(NAMESPACE_STORAGE, NVS_READONLY, &nvsHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Error (%s) while opening NVS handle", esp_err_to_name(ret));
        return ret;
    }
    // Read string from NVS
    ret = nvs_get_str(nvsHandle, variable, value, length);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_FIRMWARE, "Variable (%s) not found  [%s]", variable, esp_err_to_name(ret));
        nvs_close(nvsHandle);
        return ret;
    }
    nvs_close(nvsHandle);
    return ESP_OK;
} /**/


/**
 * Write "value" for [variable] variable in flash
 */
esp_err_t configurationWrite(char *variable, char *value) {
    nvs_handle_t nvsHandle;
    esp_err_t ret = nvs_open(NAMESPACE_STORAGE, NVS_READWRITE, &nvsHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Error (%s) while opening NVS handle", esp_err_to_name(ret));
        return ret;
    }
    ret = nvs_set_str(nvsHandle, variable, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Error [%s] while writing (%s) to NVS", esp_err_to_name(ret), variable);
        return ret;
    }
    ret = nvs_commit(nvsHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Error [%s] committing (%s) to NVS", esp_err_to_name(ret), variable);
        return ret;
    }
    nvs_close(nvsHandle);
    return ESP_OK;
} /**/


/**
 * Initialize nvs flash memory of the device
 */
esp_err_t nvsInit() {
    // Flash init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased, retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition(NVS_PARTITION_NAME));
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Avoid the ESP_ERR_NVS_NOT_FOUND after the init
    nvs_handle nvsHandle;
    nvs_open(NAMESPACE_STORAGE, NVS_READWRITE, &nvsHandle);
    nvs_close(nvsHandle);
    return ESP_OK;
} /**/



void configurationVariableInit(const char *varName, const char *defaultValue) {
    char buffer[VAR_LEN];
    size_t bufferLength = sizeof(buffer);
    if (configurationRead(varName, buffer, &bufferLength) != ESP_OK) {
        configurationWrite((char*) varName, (char*) defaultValue);
        ESP_LOGI(TAG_FIRMWARE, "    -> Default value for '%s'", varName);
    } else {
        ESP_LOGI(TAG_FIRMWARE, "    -> Getting '%s'", varName);
    }
} /**/

/**
 * Init flash and apply default values when needed
 */
void configurationSetup(void) {
    nvsInit();
    // Reading program configuration variables
    ESP_LOGI(TAG_FIRMWARE, "Reading program configuration from flash");
    configurationVariableInit(VAR_SECURITY_KEY,         DEFAULT_SECURITY_KEY);
    configurationVariableInit(VAR_WIFI_SSID,            WIFI_SSID);
    configurationVariableInit(VAR_WIFI_PASSWORD,        WIFI_PASSWORD);
    configurationVariableInit(VAR_UPDATE_URL,           DEFAULT_FIRMWARE_URL);
    configurationVariableInit(VAR_BT_ADDRESS,           DEFAULT_BT_MAC_ADDRESS);
    configurationVariableInit(VAR_CAMERA_QUALITY,       DEFAULT_CAMERA_QUALITY);
    configurationVariableInit(VAR_CAMERA_RESOLUTION,    DEFAULT_CAMERA_RESOLUTION);
    ESP_LOGI(TAG_FIRMWARE, "Configuration completed");
} /**/


bool StrToUnsignedShort8(const char *text, uint8_t *value) {
    if (*text == '\0') {            // Check for empty string
        return false;
    }
    int num = 0;                    // Convert to integer
    while (*text) {
        char c = *text++;
        if (c < '0' || c > '9') {
            return false;           // NaN -> false
        }
        num = num * 10 + (c-'0');
    }
    if (num > UINT8_MAX) {          // Check if within unsigned short int range
        return false;               // Overflow
    }
    *value = num;
    return true;                    // Conversion successful
} /**/
