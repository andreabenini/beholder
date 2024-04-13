#ifndef SERIAL__H
#define SERIAL__H

#include "freertos/FreeRTOS.h"      //
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>


#define TAG_SERIAL          "serial"
#define BUF_SIZE            200
#define TX_PIN              GPIO_NUM_15
#define RX_PIN              GPIO_NUM_14
#define UART_ATMEL          UART_NUM_2


void serialInit();
void serialTask(void *pvParameters);
void serialTaskReceiver(char *buffer, size_t size);

void serialSend(const char* data, int len);
bool serialSendAndReceive(const char *command, uint8_t commandLen, char *result, unsigned int resultLen);


#endif
