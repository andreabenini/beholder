#ifndef HTTPD__H
#define HTTPD__H

// System includes
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include <esp_timer.h>
#include <esp_wifi.h>


// Project includes
#include "main.h"
#include "camera.h"
#include "firmware.h"
#include "serial.h"


#define TAG_HTTPD               "Webserver"
#define _STREAM_PART            "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"
#define _STREAM_CONTENT_TYPE    "multipart/x-mixed-replace;boundary=" _STREAM_PART_BOUNDARY
#define _STREAM_BOUNDARY        "\r\n--" _STREAM_PART_BOUNDARY "\r\n"
#define _STREAM_PART_BOUNDARY   "123456789000000000000987654321"

#define EVENT_TYPE_COMMAND 1
#define EVENT_TYPE_STREAM  2


#define ASYNC_REQUESTS_MAX              3           // Maximum opened connections (+1 for httpd on :80)
#define ASYNC_WORKER_TASK_PRIORITY      5           // FreeRTOS task prio
// #define ASYNC_WORKER_TASK_STACK_SIZE    2048        //
#define ASYNC_WORKER_TASK_STACK_SIZE    4096        // a 4K stack size it's probably good enough for everything (even too much maybe)

#define COMMAND_BUFFER_LEN 100
#define GENERIC_BUFFER_LEN 1000


// Typedefs
typedef esp_err_t (*httpd_req_handler_t)(httpd_req_t *req);
typedef struct {
    httpd_req_t* req;
    httpd_req_handler_t handler;
} httpd_async_req_t;

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};


// Function defines
void httpdInit();

// internal functions, called by httpdStart()
void startAsyncRequestWorkers();
void taskAsyncRequestWorker(void *p);
bool isAsyncWorkerThread(void);
esp_err_t asyncRequestSubmit(httpd_req_t *req, httpd_req_handler_t handler);
bool      asyncRequestFork(httpd_req_t *req, httpd_req_handler_t handler);
#define   asyncFork(req, function)    if (asyncRequestFork(req, function)) {return ESP_OK;}
httpd_handle_t httpServerStart();
esp_err_t      httpServerStop(httpd_handle_t server);


// httpd serving functions
esp_err_t serveRootIndex(httpd_req_t *req);
esp_err_t serveWebSocket(httpd_req_t *req);
esp_err_t serveConfiguration(httpd_req_t *req);
esp_err_t serveConfigurationShow(httpd_req_t *req);
esp_err_t serveVideoStreamJPG(httpd_req_t *req);
esp_err_t serveFirmwareUpdate(httpd_req_t *req);
esp_err_t serveReboot(httpd_req_t *req);
esp_err_t serveCommand(httpd_req_t *req);


// System configuration functions
void configurationDetect(char *buffer);             // called by serveConfiguration()
void configurationUpdate(char *key, char *value);   // called by configurationDetect()

// Utilities
esp_err_t tokenValid(httpd_req_t *req);
void urlDecode(const char *urlEncoded, char *urlDecoded);


#endif
