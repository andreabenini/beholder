#include "motor.h"


/**
 * motorInit - Init motor control circuit
 */
void motorInit() {
    // Motor control inputs to OUTPUT
    pinMode(MOTOR1_IN_1, OUTPUT);
    pinMode(MOTOR1_IN_2, OUTPUT);
    pinMode(MOTOR2_IN_3, OUTPUT);
    pinMode(MOTOR2_IN_4, OUTPUT);
    // Turn off motors - Initial state
    digitalWrite(MOTOR1_IN_1, LOW);
    digitalWrite(MOTOR1_IN_2, LOW);
    digitalWrite(MOTOR2_IN_3, LOW);
    digitalWrite(MOTOR2_IN_4, LOW);
} /**/


void motorPWM(int8_t dutyCycle, uint8_t pin1, uint8_t pin2) {
    uint8_t pwm = round(abs(2.55 * dutyCycle));
    Serial.print(pin1==MOTOR1_IN_1? '1': '2');
    Serial.print(' ');
    if (dutyCycle < 0) {        // Backward
        digitalWrite(pin1, LOW);
        analogWrite(pin2, pwm);
        Serial.print('B');
    } else {                    // Stop or forward
        digitalWrite(pin2, LOW);
        analogWrite(pin1, pwm);
        Serial.print('F');
    }
    Serial.print(' ');
    Serial.print(pwm);
} /**/


/**
 * motor - Control motor movements
 * @param motorNumber (uint8_t) 0: all motors, 1: motor1, 2: motor2
 * @param dutyCycle   (int8_t)  Duty cycle percentage.  0..100: forward, 0..-100: backward
 */
void motor(uint8_t motorNumber, int8_t dutyCycle) {
    switch (motorNumber) {
    case 0:
        motorPWM(dutyCycle, MOTOR1_IN_1, MOTOR1_IN_2);
        Serial.print(' ');
        motorPWM(dutyCycle, MOTOR2_IN_3, MOTOR2_IN_4);
        break;
    case 1:
        motorPWM(dutyCycle, MOTOR1_IN_1, MOTOR1_IN_2);
        break;
    case 2:
        motorPWM(dutyCycle, MOTOR2_IN_3, MOTOR2_IN_4);
        break;
    }
} /**/
