#include "motor.h"


// TODO: Build this
/**
 * motor - Control motor movements
 * @param motorNumber (uint8_t) 0: all motors, 1: motor1, 2: motor2
 * @param dutyCycle   (uint8_t) 0..100 duty cycle on power
 * @param forward     (bool)    true:forward, false:backward
 */
void motor(uint8_t motorNumber, int8_t dutyCycle, bool forward=true) {
    switch (motorNumber) {
    case 0:
        motor(1, dutyCycle, forward);
        motor(2, dutyCycle, forward);
        return;
    case 1:
        Serial.print(motorNumber);
        break;
    case 2:
        Serial.print(motorNumber);
        break;
    }
    
    char buffer[200];
    sprintf(buffer, "Motor:%d, duty:%d, direction:%d", motor, dutyCycle, forward);
    Serial.println(buffer);
} /**/
