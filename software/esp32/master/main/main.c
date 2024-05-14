/**
 * @name        ESP32-Cam Bot Server
 * @details     Main service for Rollie bot, it controls the entire bot and interacts with Arduino and peripherals
 *              It's also responsible for dealing with user interface and external world, barebone processing is held
 *              in place, offloading compute is mainly related to cpu intensive jobs only
 * @date        2023/02/26
 * @author      Ben
 * @copyright   GPL v3 only. See (https://www.gnu.org/licenses/gpl-3.0.txt) for details
 * @version     1.0
 * 
 * @see         Explicitly written for ESP32-Cam board
 * 
 */
#include "main.h"


// System includes
#include <esp_system.h>
#include <nvs_flash.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_log.h"


// Project includes
#include "wifi.h"
#include "httpd.h"
#include "camera.h"
#include "serial.h"




/**
 * MAIN LOOP
 */
void app_main() {
#ifdef BOOTLOADER_ONLY
    startBootloader();
#else
    // Initialize Flash and read program setup
    ESP_LOGI(TAG_APP, "Application Started");
    ESP_LOGW(TAG_APP, "System boot.  [0x%02x]", esp_reset_reason());
    configurationSetup();
    // Wifi Setup
    wifiConnect();
    if (!wifiStatus) {
        ESP_LOGE(TAG_APP, "Failed to connected with Wi-Fi, check your network Credentials\n");
        wifiStop();
        cameraFlashErrorBlink();
        PROGRAM_HALT;
        return;
    }
    // Serial UART setup
    serialInit();
    xTaskCreate(serialTask, "serial_task", 2048, NULL, 10, NULL);
    cameraStart();                      // Camera setup
    httpdInit();                        // Starting HTTP server
#endif
} /**/
