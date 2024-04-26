#include <Arduino.h>


// Program defines
#include "main.h"
#include "motor.h"
#include "display.h"


#define SYSTEM_LED 13
#define BUFFER_LEN 100
char    buffer[BUFFER_LEN],
        bufferInput[BUFFER_LEN];


const char PROGMEM CMD_VERSION[]    = "version";
const char PROGMEM CMD_HELP[]       = "help";
const char PROGMEM CMD_MOTOR1[]     = "motor";                          // [motor|m] are the same command
const char PROGMEM CMD_MOTOR2[]     = "m";
const char PROGMEM CMD_MOTOR3[]     = "m0";                             // m0 works on both motors at the same time

const char PROGMEM STR_READY[]      = "READY Type 'help' for commands";
const char PROGMEM STR_ERROR[]      = "ERROR: ";
const char PROGMEM STR_VERSION[]    = "version 1.0";
const char PROGMEM STR_HELP[]       = "Commands:\n"
                                      "  help\n"
                                      "  version\n"
                                      "  m0 -100..100 -100..100\n"      // m0 <dutyCycle M1> <dutyCycle M2>     [-100: full backward, 0: stop, 100: full forward]
                                      "  m|motor 0|1|2 -100..100";      // motor <0|1|2> <dutyCycle>            [-100: full backward, 0: stop, 100: full forward]


/**
 * commandDetect - Detect command from serial input string
 * @return (void)
 */
void commandDetect() {
    char *command;
    char spaceString[] PROGMEM = " ";
    memcpy(bufferInput, buffer, strlen(buffer)+1);
    bufferInput[strlen(buffer)] = '\0';
    command = strtok(bufferInput, spaceString);

    // cmd: <m0: bothMotors> [-100..0..100: DutyCycle Motor1] [-100..0..100: DutyCycle Motor2]
    if (!strcmp_P(command, CMD_MOTOR3)) {
        int dutyCycle1 = atoi(strtok(NULL, spaceString));
        int dutyCycle2 = atoi(strtok(NULL, spaceString));
        if (dutyCycle1 < -100 || dutyCycle1 > 100) {
            dutyCycle1 = 0;
        }
        if (dutyCycle2 < -100 || dutyCycle2 > 100) {
            dutyCycle2 = 0;
        }
        motor(1, dutyCycle1);
        Serial.print(' ');
        motor(2, dutyCycle2);
        Serial.println();

    // cmd: [motor|m] [0|1|2] [-100..0..100: DutyCycle Motor]
    } else if (!strcmp_P(command, CMD_MOTOR1) || !strcmp_P(command, CMD_MOTOR2)) {
        byte motorN    = atoi(strtok(NULL, spaceString));
        int  dutyCycle = atoi(strtok(NULL, spaceString));
        if (motorN > 2) {
            Serial.print((const __FlashStringHelper *)STR_ERROR);
            Serial.println(buffer);
        } else {
            if (dutyCycle < -100 || dutyCycle > 100) {
                dutyCycle = 0;
            }
            motor(motorN, dutyCycle);
            Serial.println();
        }

    // cmd: version
    } else if (!strcmp_P(command, CMD_VERSION)) {
        Serial.println((const __FlashStringHelper *)STR_VERSION);

    // cmd: help
    } else if (!strcmp_P(command, CMD_HELP)) {
        Serial.println((const __FlashStringHelper *)STR_HELP);

    // Error //
    } else {
        Serial.print((const __FlashStringHelper *)STR_ERROR);
        Serial.println(command);
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
 * Constructor
 */
void setup() {
    displayInit();
    displayEyes(LOGO_EYE_SLEEPY);
    Serial.begin(115200);
    Serial.println((const __FlashStringHelper *)STR_READY);
    memset(buffer,      0x00, sizeof(buffer));
    memset(bufferInput, 0x00, sizeof(bufferInput));
    pinMode(SYSTEM_LED, OUTPUT);
    blink(3);
    displayEyes(LOGO_EYE_NORMAL);
} /**/

/**
 * Main loop
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
            if (strlen(buffer) < BUFFER_LEN-1) {
                strncat(buffer, &ch, 1);
            } else {
                memset(buffer, 0x00, BUFFER_LEN);
            }
            break;
        }
    }
    delay(50);
} /**/
