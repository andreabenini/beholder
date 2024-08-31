/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#if CONFIG_BT_NIMBLE_ENABLED
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#else
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#if CONFIG_BT_NIMBLE_ENABLED
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#define ESP_BD_ADDR_STR         "%02x:%02x:%02x:%02x:%02x:%02x"
#define ESP_BD_ADDR_HEX(addr)   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
#else
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#include "esp_hidh.h"
#include "esp_hid_gap.h"


#include "main.h"


#if CONFIG_BT_HID_HOST_ENABLED
static const char * remote_device_name = CONFIG_EXAMPLE_PEER_DEVICE_NAME;
#endif // CONFIG_BT_HID_HOST_ENABLED

char *bda2str(esp_bd_addr_t bda, char *str, size_t size) {
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }
    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
} /**/


uint8_t bdacmp(uint8_t *src, esp_bd_addr_t dest) {
    for (uint8_t i=0; i<6; i++) {
        if (src[i] != dest[i]) {
            return 1;
        }
    }
    return 0;
} /**/


void hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    esp_hidh_event_t event = (esp_hidh_event_t)id;
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;
    switch (event) {
    case ESP_HIDH_OPEN_EVENT: {
        if (param->open.status == ESP_OK) {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
            ESP_LOGI(TAG_APP, ESP_BD_ADDR_STR " OPEN: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->open.dev));
            esp_hidh_dev_dump(param->open.dev, stdout);
        } else {
            ESP_LOGE(TAG_APP, " OPEN failed!");
        }
        break;
    }
    case ESP_HIDH_BATTERY_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->battery.dev);
        ESP_LOGI(TAG_APP, ESP_BD_ADDR_STR " BATTERY: %d%%", ESP_BD_ADDR_HEX(bda), param->battery.level);
        break;
    }
    case ESP_HIDH_INPUT_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->input.dev);
        ESP_LOGI(TAG_APP, " INPUT: %8s, MAP: %2u, ID: %3u, Len: %d", 
                            esp_hid_usage_str(param->input.usage), param->input.map_index, param->input.report_id, param->input.length);
        ESP_LOG_BUFFER_HEX(TAG_APP, param->input.data, param->input.length);
        break;
    }
    case ESP_HIDH_FEATURE_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->feature.dev);
        ESP_LOGI(TAG_APP, ESP_BD_ADDR_STR " FEATURE: %8s, MAP: %2u, ID: %3u, Len: %d", ESP_BD_ADDR_HEX(bda),
                 esp_hid_usage_str(param->feature.usage), param->feature.map_index, param->feature.report_id,
                 param->feature.length);
        ESP_LOG_BUFFER_HEX(TAG_APP, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDH_CLOSE_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->close.dev);
        ESP_LOGI(TAG_APP, ESP_BD_ADDR_STR " CLOSE: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->close.dev));
        break;
    }
    default:
        ESP_LOGI(TAG_APP, "hidh_callback()  default event [%d]", event);
        break;
    }
} /**/


void hidDetectionTask(void *pvParameters) {
    size_t results_len = 0;
    esp_hid_scan_result_t *results;
    while (results_len == 0) {      // Keep it here until a peripheral is detected
        results = NULL;
        ESP_LOGI(TAG_APP, "hidDetectionTask() Scanning begin... (duration %ds)", SCAN_DURATION_SECONDS);
        //start scan for HID devices
        esp_hid_scan(SCAN_DURATION_SECONDS, &results_len, &results);
        ESP_LOGI(TAG_APP, "hidDetectionTask() Scanning end, found %u results", results_len);
    }
    ESP_LOGI(TAG_APP, "hidDetectionTask() Found [%u] results", results_len);
    esp_hid_scan_result_t *r = results;
    esp_hid_scan_result_t *cr = NULL;
    while (r) {
        printf("  -> %s " ESP_BD_ADDR_STR ", RSSI: %d, USAGE: %s, ", 
                (r->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(r->bda), r->rssi, esp_hid_usage_str(r->usage));
#if CONFIG_BT_BLE_ENABLED
        if (r->transport == ESP_HID_TRANSPORT_BLE) {
            cr = r;
            printf("APPEARANCE: 0x%04x, ", r->ble.appearance);
            printf("ADDR_TYPE: '%s', ", ble_addr_type_str(r->ble.addr_type));
        }
#endif /* CONFIG_BT_BLE_ENABLED */
#if CONFIG_BT_NIMBLE_ENABLED
        if (r->transport == ESP_HID_TRANSPORT_BLE) {
            cr = r;
            printf("APPEARANCE: 0x%04x, ", r->ble.appearance);
            printf("ADDR_TYPE: '%d', ", r->ble.addr_type);
        }
#endif /* CONFIG_BT_BLE_ENABLED */
#if CONFIG_BT_HID_HOST_ENABLED
        if (r->transport == ESP_HID_TRANSPORT_BT) {
            cr = r;
            printf("COD: %s[", esp_hid_cod_major_str(r->bt.cod.major));
            esp_hid_cod_minor_print(r->bt.cod.minor, stdout);
            printf("] srv 0x%03x, ", r->bt.cod.service);
            print_uuid(&r->bt.uuid);
            printf(", ");
            if (strncmp(r->name, remote_device_name, strlen(remote_device_name)) == 0) {
                break;
            }
        }
#endif /* CONFIG_BT_HID_HOST_ENABLED */
        printf("NAME: %s\n", r->name ? r->name : "");
        r = r->next;
    }

#if CONFIG_BT_HID_HOST_ENABLED
    // BT Demo: Device: Wireless Controller, Remote:HID Mouse Example
    // TODO:
    // esp_bd_addr_t controller = BT_CONTROLLER;
    // ESP_LOGW(TAG_APP, "%d", bdacmp((esp_bd_addr_t) BT_CONTROLLER, cr->bda));
    // ESP_LOGW(TAG_APP, "%d", bdacmp(controller, cr->bda));

    ESP_LOGI(TAG_APP, "Device: %s, Remote:%s", cr->name, remote_device_name);
    // if (cr && strncmp(cr->name, remote_device_name, strlen(remote_device_name)) == 0) {
    if (cr && !bdacmp((esp_bd_addr_t) BT_CONTROLLER, cr->bda)) {
        ESP_LOGI(TAG_APP, "esp_hidh_dev_open()");
        esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
    } else {
        ESP_LOGE(TAG_APP, "Device " ESP_BD_ADDR_STR " unknown", ESP_BD_ADDR_HEX(cr->bda));
    }
#else
    if (cr) {
        //open the last result
        esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
    }
#endif // CONFIG_BT_HID_HOST_ENABLED
    //free the results
    esp_hid_scan_results_free(results);
    ESP_LOGI(TAG_APP, "Closing hidDetectionTask()");
    vTaskDelete(NULL);
} /**/


#if CONFIG_BT_NIMBLE_ENABLED
void ble_hid_host_task(void *param) {
    ESP_LOGI(TAG_APP, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}
void ble_store_config_init(void);
#endif


void app_main(void) {
    char bda_str[18] = {0};
    esp_err_t ret;
    ESP_LOGW(TAG_APP, "Application started");
#if HID_HOST_MODE == HIDH_IDLE_MODE
    ESP_LOGE(TAG_APP, "Please turn on BT HID host or BLE!");
    return;
#endif
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK( esp_hid_gap_init(HID_HOST_MODE) );
#if CONFIG_BT_BLE_ENABLED
    ESP_ERROR_CHECK( esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler) );
#endif /* CONFIG_BT_BLE_ENABLED */
    esp_hidh_config_t config = {
        .callback = hidh_callback,
        .event_stack_size = 4096,
        .callback_arg = NULL,
    };
    ESP_ERROR_CHECK( esp_hidh_init(&config) );
    ESP_LOGI(TAG_APP, "Host MAC Address [%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
#if CONFIG_BT_NIMBLE_ENABLED
    /* Need to have template for store */
    ble_store_config_init();
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
	/* Starting nimble task after gatts is initialized*/
    ret = esp_nimble_enable(ble_hid_host_task);
    if (ret) {
        ESP_LOGE(TAG_APP, "esp_nimble_enable() failed: %d", ret);
    }
#endif
    // Create task for HID detection and connection
    xTaskCreate(&hidDetectionTask, "hidTask", 6 * 1024, NULL, 2, NULL);
} /**/
