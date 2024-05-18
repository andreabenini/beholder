#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"


#define BLINK_GPIO      GPIO_NUM_8


void blink(void *pvParameter) {
    // Configure the GPIO pin as output
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (true) {
        // Blink the LED on and off
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} /**/


void app_main() {
    nvs_flash_init();
    xTaskCreate(&blink, "helloLED", 2048, NULL, 5, NULL);
} /**/
