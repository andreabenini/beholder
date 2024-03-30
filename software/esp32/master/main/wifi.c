#include "wifi.h"

uint8_t             wifiStatus = 0;         // Current wifi status (bool)
uint8_t             wifiRetry  = 0;         // Current wifi retry (0..WIFI_MAX_RETRIES)
EventGroupHandle_t  wifiEventGroup;         // FreeRTOS event group to signal when we are connected


/**
 * The event group allows multiple bits for each event, but we only care about two events:
 *      (IP_EVENT_STA_GOT_IP) Connected to the AP with an IP
 *      (WIFI_BIT_FAIL)       Failed to connect after the maximum amount of retries
 */
#define WIFI_BIT_CONNECTED  BIT0
#define WIFI_BIT_FAIL       BIT1


static void wifiEvent(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifiRetry < WIFI_MAX_RETRIES) {
            ESP_LOGW(TAG_WIFI, "Retrying AP connection (SSID:%s)", WIFI_SSID);
            wifiRetry++;
            vTaskDelay(WIFI_RETRY_TIMEOUT / portTICK_PERIOD_MS);
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(wifiEventGroup, WIFI_BIT_FAIL);
        }
        wifiStatus = 0;
        ESP_LOGE(TAG_WIFI, "AP connection failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "IP:" IPSTR, IP2STR(&event->ip_info.ip));
        wifiRetry = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_BIT_CONNECTED);
        wifiStatus = 1;
    }
} /**/


/**
 * wifiConnect() Connect to wifi network, if possible
 * @see wifiStatus (uint8_t) Set accordingly from wifiEvent() function handler (0: Not Connected, 1: Connected)
 */
void wifiConnect(void) {
    wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,    &wifiEvent, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &wifiEvent, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG_WIFI, "Started");

    /* Waiting until either the connection is established (WIFI_BIT_CONNECTED) or connection failed for the maximum
     * number of re-tries (WIFI_BIT_FAIL). The bits are set by wifiEvent() (see above) */
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_BIT_CONNECTED | WIFI_BIT_FAIL, pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened */
    if (bits & WIFI_BIT_CONNECTED) {
        ESP_LOGI(TAG_WIFI, "Connected to SSID:%s password:xxxx", WIFI_SSID);
    } else if (bits & WIFI_BIT_FAIL) {
        ESP_LOGE(TAG_WIFI, "Failed to connect to SSID:%s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG_WIFI, "Unknown event: %lu", bits);
    }
    vEventGroupDelete(wifiEventGroup);
} /**/


/**
 * Turn off wireless card(s)
 */
void wifiStop(void) {
    esp_wifi_stop();
} /**/
