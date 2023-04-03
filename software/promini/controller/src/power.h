#ifndef _POWER__H_
#define _POWER__H_
#include <Arduino.h>

#define INPUT_PROBE     A0
#define OUTPUT_RELAY    13


void powerInit();
bool powerDetected();
void powerExternal(bool activate);


void dot(); // TODO: Remove

#endif