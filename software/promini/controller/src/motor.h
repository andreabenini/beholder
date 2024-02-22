#ifndef _MOTOR__H_
#define _MOTOR__H_
#include <Arduino.h>


// Motor control input pinout
#define MOTOR1_IN_1     9
#define MOTOR1_IN_2     6
#define MOTOR2_IN_3     5
#define MOTOR2_IN_4     3


void motorInit();
void motor(uint8_t motorNumber, int8_t dutyCycle);

#endif