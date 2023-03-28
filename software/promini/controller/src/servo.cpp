#include "servo.h"

Servo camServo;

void servoInit() {
    camServo.attach(10);
    delay(1000);
    camServo.write(10);
    delay(100);
    camServo.write(100);
} /**/


void servoMove(uint8_t angle) {
    angle = angle > 180? 180: angle;
    camServo.write(angle);
} /**/
