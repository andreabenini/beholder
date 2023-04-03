#include "power.h"

bool externalPowered;
bool relayON;       // TODO: REMOVE THIS ONE


/**
 * Init I/O for power detection
 */
void powerInit() {
    externalPowered = false;
    pinMode(INPUT_PROBE, INPUT);                    // Probe
    pinMode(OUTPUT_RELAY, OUTPUT);                  // Relay In
} /**/


/**
 * Detects if external power has been supplied
 */
bool powerDetected() {
    int   voltage = analogRead(INPUT_PROBE);        // Read external voltage from INPUT_PROBE pin
    float voltageValue = voltage * (5.0 / 1023.0);  // convert analog reading to voltage value
    
    Serial.print(voltageValue);    Serial.print(" ");       // DEBUG:

    if (voltageValue > 3) {                         // External power detected, something > 3V
        return true;
    }
    return false;
} /**/


/**
 * Turn ON/OFF external voltage relay to supply power to the bot while charging batteries
 * @param activate (bool) true: Turn on external relay,  false: Turn off external relay
 * @see   Do this action once, I don't need to constantly activate or toggle the relay status
 * @see   When LED is OFF the relay is ON, when the LED is ON the relay is OFF
 */
void powerExternal(bool activate) {
    if (activate==externalPowered)
        return;
    
    return; // DEBUG:

    externalPowered = activate;
    if (activate) {
        relayON = true;                             // DEBUG:
        digitalWrite(OUTPUT_RELAY, HIGH);
    } else {
        relayON = false;                            // DEBUG:
        digitalWrite(OUTPUT_RELAY, LOW);
    }
} /**/


void dot() {    // TODO: Remove this function
    if (relayON) {
        Serial.print("+");
    } else {
        Serial.print("-");
    }
}