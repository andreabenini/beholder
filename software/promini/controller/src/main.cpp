#include <Arduino.h>


// Program defines
#include "motor.h"


#define SYSTEM_LED 13
#define BUFFER_LEN 100
char    buffer[BUFFER_LEN],
        commandString[BUFFER_LEN];
bool    commandDetected;


/**
 * commandDetect - Detect command from serial input string
 * @return (void)
 */
void commandDetect() {
    if (commandDetected) {
        char *command;
        command = strtok(commandString, " ");
        if (!strcmp(command, "version")) {
            Serial.println("Version 1.0");

        } else if (!strcmp(command, "help")) {
            Serial.println("Available commands:");
            Serial.println("help\r\nversion");

        } else if (!strcmp(command, "cam")) {
            uint8_t angle = atoi(strtok(NULL, " "));
            if (angle > 0) {
                // servoMove(angle);
                Serial.println("ok");
            } else {
                Serial.println("ko");
            }

        } else if (!strcmp(command, "temp")) {
            Serial.println("TEMP command detected");
        }
        memset(commandString, 0x00, sizeof(commandString));
        commandDetected = false;
    }
} /**/


/**
 * Arduino constructor
 */
void setup() {
    Serial.begin(9600);
    memset(buffer,        0x00, sizeof(buffer));
    memset(commandString, 0x00, sizeof(commandString));
    commandDetected = false;
    // servoInit();
    // powerInit();
    pinMode(SYSTEM_LED, OUTPUT);
} /**/


/**
 * Arduino loop
 */
void loop() {
    // Serial port command detection
    while (Serial.available() && !commandDetected) {
        int byte = Serial.read();
        if (byte > 0) {
            if (byte == 0x0D) {
                commandDetected = true;
                memcpy(commandString, buffer, strlen(buffer)+1);
                memset(buffer, 0x00, sizeof(buffer));
            } else {
                char ch = (char)byte;
                strncat(buffer, &ch, 1);
            }
        }
    }
    commandDetect();


    digitalWrite(SYSTEM_LED, HIGH);   // turn the LED on
    delay(100);               // wait for a second
    digitalWrite(SYSTEM_LED, LOW);    // turn the LED off
    delay(100);     

    // powerExternal(powerDetected());

    // dot();  // FIXME: remove when done, let's also see if delay below is strictly needed
    // delay(500);
} /**/
