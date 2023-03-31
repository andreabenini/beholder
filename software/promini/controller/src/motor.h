#ifndef _MOTOR__H_
#define _MOTOR__H_
#include <Arduino.h>


void motor(uint8_t motorNumber, int8_t dutyCycle, bool forward=true);

#endif