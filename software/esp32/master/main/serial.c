#include "serial.h"

bool serialTaskRunning;
unsigned int receiveBufferSize;
char *receiveBuffer;


/**
 * Create serial port 2 on RX_PIN/TX_PIN
 */
void serialInit() {
    receiveBuffer = NULL;
    receiveBufferSize = 0;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_ATMEL, &uart_config);
    uart_set_pin(UART_ATMEL, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_ATMEL, BUF_SIZE * 2, 0, 0, NULL, 0);
} /**/

void serialTaskStop() {
    serialTaskRunning = false;
}


/**
 * Detect if a command is complete (0x0D at the end of it)
 * @return NULL  0x0D is not found and command has not been fully received
 * @return *str  Command is complete and *str is pointing to 0x0D
 */
char *isCommandTerminated(const char *str) {
    ESP_LOGI(TAG_SERIAL, "cmd (%s)", str);
    while (*str != '\0') {
        if (*str == 0x0D) {
            return (char*)str;
        }
        str++;
    }
    return NULL; // Character not found
} /**/


/**
 * Create a task only for dealing with the new serial port
 */
void serialTask(void *pvParameters) {
    serialTaskRunning = true;
    size_t bufferSize;
    char   buffer[BUF_SIZE];
    char   *terminated, *p = buffer;
    while (serialTaskRunning) {
        uart_get_buffered_data_len(UART_ATMEL, &bufferSize);
        // Buffer overflow check
        if (p-buffer+bufferSize >= BUF_SIZE) {
            memset(buffer, 0x00, BUF_SIZE);
            p = buffer;
        }
        // I've something to evaluate, let's add it to the buffer
        if (bufferSize > 0) {
            serialTaskReceiver(p, bufferSize);
            terminated = isCommandTerminated(buffer);
            if (terminated == NULL) {   // No 0x0D, carry on
                p += bufferSize;
            } else {                    // Get it, process the command and keep remainder in the buffer
                // receive [buffer] available
                *terminated = '\0';
                ESP_LOGI(TAG_SERIAL, "BUFFER RECEIVE (%s)", buffer);
                // copying buffer and unlocking serialSendAndReceive()
                if (receiveBufferSize>0) {
                    memcpy(receiveBuffer, buffer, receiveBufferSize);
                    receiveBuffer[receiveBufferSize-1] = '\0';
                    receiveBufferSize = 0;
                }
                // Moving buffer and restarting all over (if needed)
                memmove(buffer, terminated+1, BUF_SIZE-(terminated-buffer+1));
                p = buffer;
                ESP_LOGI(TAG_SERIAL, "Clearing receive buffer");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
} /**/


void serialSend(const char* data, int len) {
    uart_write_bytes(UART_ATMEL, data, len);
    uart_write_bytes(UART_ATMEL, "\r", 1);
    vTaskDelay(pdMS_TO_TICKS(200));             // Delay for a while
} /**/


bool serialSendAndReceive(const char *command, uint8_t commandLen, char *result, unsigned int resultLen) {
    receiveBuffer = result;
    receiveBufferSize = resultLen;
    serialSend(command, commandLen);
    for (uint8_t i=0; i<10; i++) {              // wait a max of 2 secs (timeout) for getting a reply
        if (receiveBufferSize == 0) {
            i = 10;
        } else {
            vTaskDelay(200/portTICK_PERIOD_MS);     // 200ms
        }
    }
    if (receiveBufferSize > 0) {
        receiveBufferSize = 0;
        return false;
    }
    return true;
} /**/


void serialTaskReceiver(char *buffer, size_t size) {
    int len = uart_read_bytes(UART_ATMEL, buffer, size, portMAX_DELAY);
    *(buffer+len) = '\0';
} /**/
