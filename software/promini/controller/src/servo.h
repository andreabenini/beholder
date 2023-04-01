#ifndef _SERVO__H_
#define _SERVO__H_
#include <Arduino.h>
#include <Servo.h>


#define ANGLE_MIN 10
#define ANGLE_MAX 160

void servoInit();
void servoMove(uint8_t angle);

#endif