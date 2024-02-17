#include <Arduino.h>


// Program defines
#include "main.h"
#include "motor.h"


#define SYSTEM_LED 13
#define BUFFER_LEN 100
char    buffer[BUFFER_LEN],
        bufferInput[BUFFER_LEN];


const char PROGMEM CMD_VERSION[]    = "version";
const char PROGMEM CMD_HELP[]       = "help";
const char PROGMEM CMD_MOTOR[]      = "motor";

const char PROGMEM STR_READY[]      = "READY Type 'help' for commands";
const char PROGMEM STR_ERROR[]      = "ERROR: ";
const char PROGMEM STR_VERSION[]    = "version 1.0";
const char PROGMEM STR_HELP[]       = "Commands:\n"
                                      "  help\n"
                                      "  version\n"
                                      "  motor 0|1|2 0..100 0|1";


/**
 * commandDetect - Detect command from serial input string
 * @return (void)
 */
void commandDetect() {
    char *command;
    memcpy(bufferInput, buffer, strlen(buffer)+1);
    command = strtok(bufferInput, " ");
    // cmd: version
    if (!strcmp_P(command, CMD_VERSION)) {
        Serial.println((const __FlashStringHelper *)STR_VERSION);

    // cmd: help
    } else if (!strcmp_P(command, CMD_HELP)) {
        Serial.println((const __FlashStringHelper *)STR_HELP);

    // cmd: motor [0|1|2] [0..100: dutyCycle] [0|1: forward(default)|backward]
    } else if (!strcmp_P(command, CMD_MOTOR)) {
        byte motorN    = atoi(strtok(NULL, " "));
        byte dutyCycle = atoi(strtok(NULL, " "));
        byte forward   = atoi(strtok(NULL, " "));
        if (motorN > 2) {
            Serial.print((const __FlashStringHelper *)STR_ERROR);
            Serial.println(buffer);
        } else {
            if (dutyCycle > 100) {
                dutyCycle = 0;
            }
            motor(motorN, dutyCycle, forward==0? true: false);
        }

    // Error //
    } else {
        Serial.print((const __FlashStringHelper *)STR_ERROR);
        Serial.println(buffer);
    }
    memset(buffer,      0x00, sizeof(buffer));
    memset(bufferInput, 0x00, sizeof(bufferInput));
} /**/

void blink(int8_t times) {
    for (int8_t i=0; i<times; i++) {
        digitalWrite(SYSTEM_LED, HIGH);
        delay(500);
        digitalWrite(SYSTEM_LED, LOW);
        delay(500);     
    }
} /**/

/**
 * Arduino constructor
 */
void setup() {
    Serial.begin(9600);
    Serial.println((const __FlashStringHelper *)STR_READY);
    memset(buffer,      0x00, sizeof(buffer));
    memset(bufferInput, 0x00, sizeof(bufferInput));
    pinMode(SYSTEM_LED, OUTPUT);
    blink(3);
} /**/

/**
 * Arduino loop
 */
void loop() {
    // Serial port command detection
    while (Serial.available()) {
        byte byte = Serial.read();
        switch (byte) {
        case 0:
        case 0x0A:
            break;
        case 0x0D:
            commandDetect();
            break;
        default:
            char ch = (char)byte;
            strncat(buffer, &ch, 1);
            break;
        }
    }
    delay(500);
} /**/
