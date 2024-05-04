#include "httpd.h"

QueueHandle_t     asyncRequestQueue;
SemaphoreHandle_t workerReadyCount;
TaskHandle_t      workerHandles[ASYNC_REQUESTS_MAX];
bool              isStreamingVideo;


/**
 * Start an async worker task on FreeRTOS
 */
void taskAsyncRequestWorker(void *p) {
    ESP_LOGI(TAG_HTTPD, "Starting async request task worker");
    while (true) {
        // counting semaphore. This signals that a worker is ready to accept work
        xSemaphoreGive(workerReadyCount);
        // wait for a request
        httpd_async_req_t async_req;
        if (xQueueReceive(asyncRequestQueue, &async_req, portMAX_DELAY)) {
            async_req.handler(async_req.req);           // Call the handler
            // Inform the server that it can purge the socket used for this request, if needed.
            if (httpd_req_async_handler_complete(async_req.req) != ESP_OK) {
                ESP_LOGE(TAG_HTTPD, "failed to complete async req");
            }
        }
    }
    // Tasks are automatically purged from the OS once done. double check before deleting these two lines
    ESP_LOGW(TAG_HTTPD, "async request worker stopped");
    vTaskDelete(NULL);
} /**/


/**
 * Starting the workers queue for managing tasks
 */
void startAsyncRequestWorkers() {
    // counting semaphore keeps track of available workers
    workerReadyCount = xSemaphoreCreateCounting(
                ASYNC_REQUESTS_MAX,     // Max Count
                0);                     // Initial Count
    if (workerReadyCount == NULL) {
        ESP_LOGE(TAG_HTTPD, "Failed to create workers counting Semaphore");
        return;
    }
    // create queue
    asyncRequestQueue = xQueueCreate(1, sizeof(httpd_async_req_t));
    if (asyncRequestQueue == NULL){
        ESP_LOGE(TAG_HTTPD, "Failed to create async_req_queue");
        vSemaphoreDelete(workerReadyCount);
        return;
    }
    // start worker tasks
    for (int i = 0; i < ASYNC_REQUESTS_MAX; i++) {
        bool success = xTaskCreate(taskAsyncRequestWorker, "async_req_worker",
                                   ASYNC_WORKER_TASK_STACK_SIZE,    // stack size
                                   (void *)0,                       // argument, (none here)
                                   ASYNC_WORKER_TASK_PRIORITY,      // priority
                                   &workerHandles[i]);
        if (!success) {
            ESP_LOGE(TAG_HTTPD, "Failed to start asyncReqWorker");
            continue;
        }
    }
} /**/

/**
 * Is our handle one of the known async handles?
 */
bool isAsyncWorkerThread(void) {
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    for (int i = 0; i < ASYNC_REQUESTS_MAX; i++) {
        if (workerHandles[i] == handle) {
            return true;
        }
    }
    return false;
} /**/


esp_err_t asyncRequestSubmit(httpd_req_t *req, httpd_req_handler_t handler) {
    // must create a copy of the request that we own
    httpd_req_t* copy = NULL;
    esp_err_t err = httpd_req_async_handler_begin(req, &copy);
    if (err != ESP_OK) {
        return err;
    }
    httpd_async_req_t asyncRequest = {
        .req = copy,
        .handler = handler,
    };
    // What resource exhaustion? here we immediately respond with an http error if no workers are available
    int ticks = 0;
    // counting semaphore: if success, we know 1 or more asyncReqTaskWorkers are available.
    if (xSemaphoreTake(workerReadyCount, ticks) == false) {
        ESP_LOGE(TAG_HTTPD, "No workers are available");
        httpd_req_async_handler_complete(copy); // cleanup
        return ESP_FAIL;
    }
    // Since workerReadyCount > 0 the queue should already have space but lets wait up to 100ms just to be safe
    if (xQueueSend(asyncRequestQueue, &asyncRequest, pdMS_TO_TICKS(100)) == false) {
        ESP_LOGE(TAG_HTTPD, "worker queue is full");
        httpd_req_async_handler_complete(copy); // cleanup
        return ESP_FAIL;
    }
    return ESP_OK;
} /**/

/**
 * Handler is first invoked on the httpd thread. In order to free main thread to handle
 * other requests, this request has to be resubmitted and handled on an async worker thread
 */
bool asyncRequestFork(httpd_req_t *req, httpd_req_handler_t handler) {
    if (isAsyncWorkerThread() == false) {
        ESP_LOGI(TAG_HTTPD, "[main] %s '%s'", http_method_str(req->method), req->uri);
        if (asyncRequestSubmit(req, handler) == ESP_OK) {
            return true;
        } else {
            httpd_resp_set_status(req, "503 Busy");
            httpd_resp_sendstr(req, "<div>No workers available. Server busy</div>");
            return true;
        }
    }
    ESP_LOGI(TAG_HTTPD, "[FORK] %s '%s'", http_method_str(req->method), req->uri);
    return false;
} /**/


/**
 * Initialize and start the builtin HTTPD server
 * @see Do not use this directly, called by httpdStart()
 */
httpd_handle_t httpServerStart() {
    isStreamingVideo = false;
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    /*
        It is advisable that httpd_config_t->max_open_sockets > MAX_ASYNC_REQUESTS.
        This leaves at least one socket available to handle quick synchronous requests.
        Otherwise, all the sockets will get taken by the long async handlers and server
        will no longer be responsive.
    */ config.max_open_sockets = ASYNC_REQUESTS_MAX+1;
    // Start the http server
    ESP_LOGI(TAG_HTTPD, "Starting server on port [%d]", config.server_port);
    if (httpd_start(&server , &config) != ESP_OK) {
        ESP_LOGE(TAG_HTTPD, "Error starting http server");
        return NULL;
    }
    const httpd_uri_t uriGetRoot = {
        .uri      = "/",
        .method   = HTTP_GET,
        .handler  = serveRootIndex,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriGetRoot);
    const httpd_uri_t uriGetWebSocket = {
        .uri      = "/ws",
        .method   = HTTP_GET,
        .handler  = serveWebSocket,
        .is_websocket = true,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriGetWebSocket);
    const httpd_uri_t uriGetVideo = {
        .uri      = "/video",
        .method   = HTTP_GET,
        .handler  = serveVideoStreamJPG,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriGetVideo);
    const httpd_uri_t uriGetConfiguration = {
        .uri      = "/config",
        .method   = HTTP_GET,
        .handler  = serveConfigurationShow,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriGetConfiguration);
    const httpd_uri_t uriPostConfiguration = {
        .uri      = "/config",
        .method   = HTTP_POST,
        .handler  = serveConfiguration,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriPostConfiguration);
    const httpd_uri_t uriUploadFirmware = {
        .uri      = "/firmware",
        .method   = HTTP_GET,
        .handler  = serveFirmwareUpdate,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriUploadFirmware);
    const httpd_uri_t uriReboot = {
        .uri      = "/reboot",
        .method   = HTTP_GET,
        .handler  = serveReboot,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriReboot);
    const httpd_uri_t uriCommand = {
        .uri      = "/cmd",
        .method   = HTTP_GET,
        .handler  = serveCommand,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uriCommand);
    return server;
} /**/

/**
 * Stop the httpd server
 */
esp_err_t httpServerStop(httpd_handle_t server) {
    return httpd_stop(server);
} /**/


/**
 * Serve "/video"
 */
esp_err_t serveVideoStreamJPG(httpd_req_t *req) {
    if (isStreamingVideo) {
        ESP_LOGW(TAG_HTTPD, "Camera is already streaming video, aborting a new stream");
        return ESP_FAIL;
    }
    asyncFork(req, serveVideoStreamJPG);
    // // validate 'token' in the Headers
    // if (tokenValid(req) != ESP_OK) {
    //     httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
    //     return ESP_FAIL;
    // }
    isStreamingVideo = true;
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }
    while (isStreamingVideo) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG_HTTPD, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if (fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if (!jpeg_converted) {
                ESP_LOGE(TAG_HTTPD, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb->format != PIXFORMAT_JPEG) {
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if (res != ESP_OK) {
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGD(TAG_HTTPD, "MJPG %luKb %02.0ffps %lums", (uint32_t)(_jpg_buf_len/1024), 1000.0/(uint32_t)frame_time, (uint32_t)(frame_time));
    }
    isStreamingVideo = false;
    ESP_LOGI(TAG_HTTPD, "Video streaming stopped");
    last_frame = 0;
    httpd_resp_send_chunk(req, "--" _STREAM_PART_BOUNDARY "--", -1);        // Send terminating boundary
    httpd_resp_send_chunk(req, NULL, last_frame);                           // Send final empty chunk to indicate end of response
    return ESP_OK;
}

/**
 * Serve "/"
 */
esp_err_t serveRootIndex(httpd_req_t *req) {
    asyncFork(req, serveRootIndex);
    ESP_LOGI(TAG_HTTPD, "       providing empty page information on root");
    const char *dummyReply = "<html><body></body></html>\n";
    esp_err_t res = httpd_resp_set_type(req, "text/html");
    if (res != ESP_OK) {
        return res;
    }
    httpd_resp_send(req, dummyReply, strlen(dummyReply));
    return ESP_OK;
} /**/


/**
 * Serve web socket control commands
 * This handler echos back the received ws data and triggers an async send if certain message received
 */
esp_err_t serveWebSocket(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        asyncFork(req, serveWebSocket);
        // // validate 'token' in the Headers
        // if (tokenValid(req) != ESP_OK) {
        //     httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        //     return ESP_FAIL;
        // }
        ESP_LOGI(TAG_HTTPD, "    WS: Handshake completed, new connection opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG_HTTPD, "  WS:   Client request started");
    httpd_ws_frame_t ws_pkt;
    uint8_t          *buffer = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    // Set max_len=0 to get the frame len
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_HTTPD, "WS: httpd_ws_recv_frame() failed to get frame len (%d)", ret);
        return ret;
    }
    ESP_LOGI(TAG_HTTPD, "    WS: Frame length: %d", ws_pkt.len);
    ESP_LOGI(TAG_HTTPD, "    WS: Type: %d", ws_pkt.type);
    if (ws_pkt.len) {
        // ws_pkt.len+1 is for NULL termination, expecting a string
        buffer = calloc(1, ws_pkt.len + 1);
        if (buffer == NULL) {
            ESP_LOGE(TAG_HTTPD, "WS: Failed to calloc() memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buffer;
        // max_len=ws_pkt.len to get the frame payload
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_HTTPD, "WS: httpd_ws_recv_frame() failed (%d)", ret);
            free(buffer);
            return ret;
        }
        ESP_LOGI(TAG_HTTPD, "    WS: < '%s'", ws_pkt.payload);
    }
    // ["m <m1Duty> <m2Duty>"] Motor Control
    if (ws_pkt.type==HTTPD_WS_TYPE_TEXT && !strncmp((char*)ws_pkt.payload,"m ", 2)) {
        // Getting motors dutycycles
        short int m1, m2;
        char *token;
        token = strtok((char*)buffer, " ");
        token = strtok(NULL, " ");
        m1 = atoi(token);
        token = strtok(NULL, " ");
        m2 = atoi(token);
        free(buffer);
        // Sending command and sending back results
        char bufferReceive[COMMAND_BUFFER_LEN];
        sprintf(bufferReceive, "m0 %d %d", m1, m2);
        serialSendAndReceive(bufferReceive, strlen(bufferReceive), bufferReceive, COMMAND_BUFFER_LEN);
        ws_pkt.payload = (uint8_t*)bufferReceive;
        ws_pkt.len = strlen(bufferReceive);
        ret = httpd_ws_send_frame(req, &ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_HTTPD, "WS: httpd_ws_send_frame() failed with %d", ret);
        }
        return ret;
    }
    // ["status"] Report system status
    if (ws_pkt.type==HTTPD_WS_TYPE_TEXT && !strcmp((char*)ws_pkt.payload,"status")) {
        free(buffer);
        char bufferReceive[COMMAND_BUFFER_LEN];
        serialSendAndReceive("version", 7, bufferReceive, COMMAND_BUFFER_LEN);
        ws_pkt.payload = (uint8_t*)bufferReceive;
        ws_pkt.len = strlen(bufferReceive);
        ret = httpd_ws_send_frame(req, &ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_HTTPD, "WS: httpd_ws_send_frame() failed with %d", ret);
        }
        return ret;
    }
    
    // Unknown command: Error
    free(buffer);
    uint8_t cErrorMsg[4];
    sprintf((char*)cErrorMsg, "ERR");
    ws_pkt.payload = cErrorMsg;
    ESP_LOGI(TAG_HTTPD, "    WS: > '%s'", ws_pkt.payload);
    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_HTTPD, "WS: httpd_ws_send_frame() failed with %d", ret);
    }
    return ret;
} /**/


/**
 * Serve "/firmware"   Update firmware from remote URL
 */
esp_err_t serveFirmwareUpdate(httpd_req_t *req) {
    // Content-type
    if (httpd_resp_set_type(req, "text/plain") != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_501_METHOD_NOT_IMPLEMENTED, "Cannot set output Content-Type\n");
        return ESP_FAIL;
    }
    // validate 'token' in the Headers
    if (tokenValid(req) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        return ESP_FAIL;
    }
    // Update the firmware
    const char *dummyReply = "\nUpdating system firmware\nPlease Standby :)\n\n";
    httpd_resp_send(req, dummyReply, strlen(dummyReply));
    firmwareUpdate();
    return ESP_OK;
} /**/

/**
 * Serve "/config"   Update software configuration
 */
esp_err_t serveConfiguration(httpd_req_t *req) {
    // validate 'token' in the Headers
    if (tokenValid(req) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        return ESP_FAIL;
    }
    static char buffer[GENERIC_BUFFER_LEN];
    uint16_t lenTotal = req->content_len;
    uint16_t lenCurrent = 0;
    uint16_t bytesReceived = 0;
    if (lenTotal >= GENERIC_BUFFER_LEN) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Data invalid\n");
        return ESP_FAIL;
    }
    // Retrieve POST HTTP request
    if (isAsyncWorkerThread() == false) {
        ESP_LOGI(TAG_HTTPD, "[main] %s '%s'", http_method_str(req->method), req->uri);
        memset(buffer, 0x00, GENERIC_BUFFER_LEN);
        while (lenCurrent < lenTotal) {
            int ret = httpd_req_recv(req, buffer+bytesReceived, lenTotal);
            if (ret <= 0) {     // 0 return value indicates connection closed
                ESP_LOGE(TAG_HTTPD, "Invalid request httpd_req_recv() [ret=%d]", ret);
                if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                    ESP_LOGI(TAG_HTTPD, "[%d, %d] Retry: %d", bytesReceived, lenTotal, httpd_req_recv(req, buffer+bytesReceived, lenTotal) );
                    httpd_resp_send_err(req, HTTPD_408_REQ_TIMEOUT, "Request timeout\n");
                } else {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request\n");
                }
                return ESP_FAIL;
            }
            lenCurrent += ret;
            bytesReceived += ret;
            buffer[bytesReceived] = '\0';   // Null-terminate the received data
        }
        // Now serving request in a different thread (BELOW) and let this return to main loop
        if (asyncRequestSubmit(req, serveConfiguration) == ESP_OK) {
            return ESP_OK;
        } else {
            httpd_resp_set_status(req, "503 Busy");
            httpd_resp_sendstr(req, "<div>No workers available. Server busy</div>");
            return ESP_FAIL;
        }
    }
    ESP_LOGI(TAG_HTTPD, "[FORK] %s '%s'   Payload:(%s)", http_method_str(req->method), req->uri, buffer);
    configurationDetect(buffer);
    ESP_LOGI(TAG_HTTPD, "Configuration updated");
    const char *dummyReply = "Configuration updated\n";
    esp_err_t res = httpd_resp_set_type(req, "text/html");
    if (res != ESP_OK) {
        return res;
    }
    httpd_resp_send(req, dummyReply, strlen(dummyReply));
    return ESP_OK;
} /**/

esp_err_t serveConfigurationShow(httpd_req_t *req) {
    asyncFork(req, serveConfigurationShow);
    // validate 'token' in the Headers
    if (tokenValid(req) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        return ESP_FAIL;
    }
    // Content-type
    if (httpd_resp_set_type(req, "text/plain") != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_501_METHOD_NOT_IMPLEMENTED, "Cannot set output Content-Type\n");
        return ESP_FAIL;
    }
    char buffer[GENERIC_BUFFER_LEN];
    char variable[VAR_LEN];
    ESP_LOGI(TAG_HTTPD, "Providing system configuration to user");
    sprintf(buffer, "\nSystem Configuration\n\n");
    size_t variableLength = sizeof(variable);
    if (configurationRead(VAR_WIFI_SSID, variable, &variableLength) == ESP_OK  &&  strlen(buffer)+VAR_LEN*2 < GENERIC_BUFFER_LEN) {
        sprintf(buffer+strlen(buffer), "    WiFi Credentials    SSID:%s  secret:xxx\n", variable);
        variableLength = sizeof(variable);
    }
    if (configurationRead(VAR_UPDATE_URL, variable, &variableLength) == ESP_OK  &&  strlen(buffer)+VAR_LEN*2 < GENERIC_BUFFER_LEN) {
        sprintf(buffer+strlen(buffer), "    Update URL          %s\n", variable);
        variableLength = sizeof(variable);
    }
    if (configurationRead(VAR_BT_ADDRESS, variable, &variableLength) == ESP_OK  &&  strlen(buffer)+VAR_LEN*2 < GENERIC_BUFFER_LEN) {
        sprintf(buffer+strlen(buffer), "    Bluetooth MAC       %s\n", variable);
        variableLength = sizeof(variable);
    }
    if (configurationRead(VAR_CAMERA_QUALITY, variable, &variableLength) == ESP_OK  &&  strlen(buffer)+VAR_LEN*2 < GENERIC_BUFFER_LEN) {
        sprintf(buffer+strlen(buffer), "    Camera Quality      %s\n", variable);
        variableLength = sizeof(variable);
    }
    if (configurationRead(VAR_CAMERA_RESOLUTION, variable, &variableLength) == ESP_OK  &&  strlen(buffer)+VAR_LEN*2 < GENERIC_BUFFER_LEN) {
        sprintf(buffer+strlen(buffer), "    Camera Resolution   %s\n", variable);
        variableLength = sizeof(variable);
    }
    sprintf(buffer+strlen(buffer), "\n");
    httpd_resp_send(req, buffer, strlen(buffer));
    return ESP_OK;
} /**/

/**
 * Serve "/reboot"
 */
esp_err_t serveReboot(httpd_req_t *req) {
    // Content-type
    if (httpd_resp_set_type(req, "text/plain") != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_501_METHOD_NOT_IMPLEMENTED, "Cannot set output Content-Type\n");
        return ESP_FAIL;
    }
    // validate 'token' in the Headers
    if (tokenValid(req) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        return ESP_FAIL;
    }
    // Update the firmware
    ESP_LOGW(TAG_HTTPD, "Rebooting device");
    const char *dummyReply = "\nRebooting device\nPlease wait :)\n\n";
    httpd_resp_send(req, dummyReply, strlen(dummyReply));
    vTaskDelay(2000 / portTICK_PERIOD_MS);      // 2 secs
    esp_restart();
    return ESP_OK;
} /**/

/**
 * Serve "/cmd"
 */
esp_err_t serveCommand(httpd_req_t *req) {
    asyncFork(req, serveCommand);
    // validate 'token' in the Headers
    if (tokenValid(req) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Authentication failed\n");
        return ESP_FAIL;
    }
    // Get the command from the querystring
    char *query = (char*)req->uri+strlen(req->uri);
    while(*query != '?' && query != req->uri) {
        query--;
    }
    if (query == req->uri) {
        ESP_LOGI(TAG_HTTPD, "No query parameters found");
        httpd_resp_send_404(req);
        return ESP_OK;
    }
    query++;            // Skip '?' character
    char command[COMMAND_BUFFER_LEN];
    urlDecode(query, command);
    // Send command to serial port
    ESP_LOGI(TAG_HTTPD, "cmd> %s", command);
    serialSend(command, strlen(command));
    // Reply back to http client
    const char *dummyReply = "ok\n";
    httpd_resp_send(req, dummyReply, strlen(dummyReply));
    return ESP_OK;
} /**/

/**
 * Function to decode URL-encoded string
 */
void urlDecode(const char *urlEncoded, char *urlDecoded) {
    char hex[3] = {0};
    int j = 0;
    for (int i = 0; i < strlen(urlEncoded); i++) {
        if (urlEncoded[i] == '%') {
            hex[0] = urlEncoded[++i];
            hex[1] = urlEncoded[++i];
            urlDecoded[j++] = strtol(hex, NULL, 16);
        } else if (urlEncoded[i] == '+') {
            urlDecoded[j++] = ' ';
        } else {
            urlDecoded[j++] = urlEncoded[i];
        }
    }
    urlDecoded[j] = '\0';
}

/**
 * Update program configuration
 */
void configurationUpdate(char *key, char *value) {
    ESP_LOGI(TAG_HTTPD, "      Var: '%s', Value: %s", key, value);
    if (// Security token
        !strcmp(key, VAR_SECURITY_KEY)      ||
        // WiFi configuration
        !strcmp(key, VAR_WIFI_SSID)         ||
        !strcmp(key, VAR_WIFI_PASSWORD)     ||
        // Bluetooth configuration
        !strcmp(key, VAR_BT_ADDRESS)        ||
        // Camera settings
        !strcmp(key, VAR_CAMERA_QUALITY)    ||
        !strcmp(key, VAR_CAMERA_RESOLUTION) ||
        // update url
        !strcmp(key, VAR_UPDATE_URL)        ) {

            char buffer[VAR_LEN];
            urlDecode(value, buffer);
            ESP_LOGI(TAG_HTTPD, "           '%s' -> '%s'", value, buffer);
            configurationWrite(key, buffer);
    }
    if (!strcmp(key, VAR_CAMERA_QUALITY)    ||
        !strcmp(key, VAR_CAMERA_RESOLUTION) ) {
            ESP_LOGI(TAG_HTTPD, "Camera setup changed, reinitializing with new settings");
            isStreamingVideo = false;
            vTaskDelay(2000 / portTICK_PERIOD_MS);      // 2 secs
            cameraStart();
    }
} /**/

/**
 * Split HTTP buffer and detect configuration by creating a var,value keypair
 */
void configurationDetect(char *buffer) {
    ESP_LOGI(TAG_HTTPD, "Received data via POST method: %s", buffer);
    // Parse and print key-value pairs
    char *token = strtok(buffer, "&");
    while (token != NULL) {
        ESP_LOGI(TAG_HTTPD, "Parameter: %s", token);
        char *separator = strchr(token, '=');
        if (separator != NULL) {
            *separator = '\0';      // null-terminate key
            char *key = token;
            char *value = separator + 1;
            configurationUpdate(key, value);
        }
        token = strtok(NULL, "&");
    }
} /**/

/**
 * Validate token in the http header, if any
 * @return ESP_OK     on success
 * @return ESP_FAIL   on failure
 */
esp_err_t tokenValid(httpd_req_t *req) {
    char tokenCurrent[VAR_LEN+1],
         tokenRequest[VAR_LEN+1];
    size_t tokenLength;

    // Get user token from http request [tokenRequest]
    tokenLength = httpd_req_get_hdr_value_len(req, "token")+1;
    tokenCurrent[0] = '\0';
    if (tokenLength <= 1 || httpd_req_get_hdr_value_str(req, "token", tokenCurrent, tokenLength) != ESP_OK) {
        ESP_LOGE(TAG_HTTPD, "Header 'token' not found");
        return ESP_FAIL;
    }
    urlDecode(tokenCurrent, tokenRequest);
    // Get system secret from flash [tokenCurrent]
    tokenLength = sizeof(tokenCurrent);
    if (configurationRead(VAR_SECURITY_KEY, tokenCurrent, &tokenLength) != ESP_OK) {
        ESP_LOGE(TAG_FIRMWARE, "Cannot get '%s' value", VAR_SECURITY_KEY);
        return ESP_FAIL;
    }

    if (!strcmp(tokenCurrent, tokenRequest)) {
        ESP_LOGI(TAG_HTTPD, "Request has been validated");
        return ESP_OK;
    }
    ESP_LOGE(TAG_HTTPD, "Request denied, invalid token (%s). Should be (%s)", tokenRequest, tokenCurrent);
    return ESP_FAIL;
} /**/


void handlerConnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG_HTTPD, "Starting webserver");
        *server = httpServerStart();
    }
} /**/

void handlerDisconnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG_HTTPD, "Stopping webserver");
        if (httpServerStop(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG_HTTPD, "Failed to stop http server");
        }
    }
} /**/


/**
 * Start the http daemon and create an event queue for it
 * @see Call this function as the main (and only) entry point for everything
 */
void httpdInit() {
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP,         &handlerConnect,    &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &handlerDisconnect, &server));
    server = httpServerStart();
    if (server == NULL) {
        ESP_LOGE(TAG_APP, "Cannot start web Server");
    } else {
        ESP_LOGI(TAG_APP, "ESP32 CAM Web Server is up and running");
        startAsyncRequestWorkers();
    }
} /**/
