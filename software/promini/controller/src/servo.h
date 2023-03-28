#ifndef _SERVO__H_
#define _SERVO__H_
#include <Arduino.h>
#include <Servo.h>


void servoInit();
void servoMove(uint8_t angle);

#endif