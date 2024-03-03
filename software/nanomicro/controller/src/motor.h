#ifndef _MOTOR__H_
#define _MOTOR__H_
#include <Arduino.h>


// Motor control input pinout
#define MOTOR1_IN_1     10
#define MOTOR1_IN_2     9
#define MOTOR2_IN_3     6
#define MOTOR2_IN_4     5


void motorInit();
void motor(uint8_t motorNumber, int8_t dutyCycle);

#endif