#include "servo.h"

Servo camServo;

void servoInit() {
    camServo.attach(10);        // I/O pin 10
    delay(1000);
    servoMove(5);
    delay(50);
    servoMove(100);
} /**/


void servoMove(uint8_t angle) {
    if (angle < ANGLE_MIN) {
        angle = ANGLE_MIN;
    } else if (angle > ANGLE_MAX) {
        angle = ANGLE_MAX;
    }
    camServo.write(angle);
} /**/
