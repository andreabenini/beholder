#ifndef WIFI__H
#define WIFI__H

// System Includes
#include <esp_system.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>


// Project includes
#include "main.h"


// Defines
#define TAG_WIFI    "WiFi"


extern uint8_t wifiStatus;
void wifiConnect(void);
void wifiStop(void);


#endif