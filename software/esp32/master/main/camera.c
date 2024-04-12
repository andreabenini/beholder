#include "camera.h"


bool cameraSetting_uint8_t(const char *varName, uint8_t *result) {
    char   buffer[VAR_LEN];
    size_t bufferLength = sizeof(buffer);
    if (configurationRead(varName, buffer, &bufferLength) != ESP_OK) {
        ESP_LOGE(TAG_CAMERA, "    Cannot get '%s' value", varName);
        return false;
    }
    if (!StrToUnsignedShort8(buffer, result)) {
        ESP_LOGE(TAG_CAMERA, "    '%s' is not a valid for %s", buffer, varName);
        return false;
    }
    return true;
} /**/


void cameraStart(void) {
    // Reading camera setup
    uint8_t quality, resolution;
    ESP_LOGI(TAG_CAMERA, "Reading camera setup values from flash");
    if (!cameraSetting_uint8_t(VAR_CAMERA_QUALITY,      &quality))          { return; }
    if (!cameraSetting_uint8_t(VAR_CAMERA_RESOLUTION,   &resolution))       { return; }
    esp_camera_deinit();
    esp_err_t err = cameraInit(quality, resolution);            // Init builtin camera module
    if (err != ESP_OK) {
        ESP_LOGE(TAG_CAMERA, "Camera init error: %s", esp_err_to_name(err));
        return;
    }
} /**/


esp_err_t cameraInit(uint8_t quality, uint8_t resolution) {
    ESP_LOGI(TAG_CAMERA, "cameraInit(%d, %d)", quality, resolution);
    camera_config_t camera_config = {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,             // Default: PIXFORMAT_JPEG
                                                    //          PIXFORMAT_RAW
                                                    //              Error, ll_cam: requested format is not supported
                                                    //          PIXFORMAT_YUV422
                                                    //              Error, HTTPD socket error (?)
                                                    //          PIXFORMAT_YUV420
                                                    //              Error, requested format is not supported
                                                    //          PIXFORMAT_GRAYSCALE
                                                    //              Error, HTTPD uri handler execution failed (error in send: 11)
                                                    //          PIXFORMAT_RGB565
                                                    //              Slow (1fps) but it works. Frame ~31Kb
                                                    //          PIXFORMAT_RGB888, PIXFORMAT_RGB444, PIXFORMAT_RGB555
                                                    //              Crashes, requested format not supported
        .frame_size     = resolution,               // Default: FRAMESIZE_VGA (FRAMESIZE_QVGA,FRAMESIZE_XGA,FRAMESIZE_UXGA)
                                                    //          (sensor.h) typedef enum framesize_t
        .jpeg_quality   = quality,                  //          10 (20Kb) Quality of JPEG output. 0-63 lower means higher quality
                                                    //          20 (12Kb) ~10 fps fb_count=1, ~20 fps fb_count=2, >20 fps fb_count=3
                                                    //          25 (11Kb) ~15 fps fb_count=2
                                                    // Default: 30 ( 8Kb) ~15 fps fb_count=1, ~10 fps fb_count=2
        .fb_count = 2,                              // Default: 1
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY         // Default: CAMERA_GRAB_WHEN_EMPTY      (CAMERA_GRAB_WHEN_EMPTY|CAMERA_GRAB_LATEST)
    };                                              //          CAMERA_GRAB_LATEST          Sets when buffers should be filled
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        return err;
    }
    return ESP_OK;
} /**/



/**
 * Blink the Flash led for a minute to report some kind of error
 */
void taskFlashError(void *pvParameters) {
    gpio_pad_select_gpio(FLASH_LED_GPIO);
    gpio_set_direction(FLASH_LED_GPIO, GPIO_MODE_OUTPUT);
    for (uint8_t i=0; i<60; i++) {
        // Turn flash LED on
        gpio_set_level(FLASH_LED_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS); // 500ms delay
        // Turn flash LED off
        gpio_set_level(FLASH_LED_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS); // 500ms delay
    }
} /**/

/**
 * Blink the flash as an error message, this function never quits
 */
void cameraFlashErrorBlink(void) {
    xTaskCreate(&taskFlashError, "taskFlashError", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
} /**/
