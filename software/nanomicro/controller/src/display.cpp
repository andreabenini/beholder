#include "display.h"

#define I2C_ADDRESS_LEFT    0x3C    // Left  eye device address
#define I2C_ADDRESS_RIGHT   0x3D    // Right eye device address
// #define I2C_ADDRESS 0x3D            // Right eye device address
#define RST_PIN     -1              // Define proper RST_PIN if required (-1 if not available/needed)

SSD1306AsciiAvrI2c oledLeft, oledRight;

void displayInit() {
#if RST_PIN >= 0
    oledLeft.begin(&Adafruit128x64,  I2C_ADDRESS_LEFT,  RST_PIN);
    oledRight.begin(&Adafruit128x64, I2C_ADDRESS_RIGHT, RST_PIN);
#else
    oledLeft.begin(&Adafruit128x64,  I2C_ADDRESS_LEFT);
    oledRight.begin(&Adafruit128x64, I2C_ADDRESS_RIGHT);
#endif
    // Call oledLeft.setI2cClock(frequency) to change from the default frequency.
    oledLeft.setFont(Adafruit5x7);
    oledRight.setFont(Adafruit5x7);
} /**/

void displayEyes(byte image) {
    // Fetching data before displaying it usually provides enhanced results
    byte xPos   = pgm_read_byte(&Logo[image].xPos);
    byte yPos   = pgm_read_byte(&Logo[image].yPos);
    byte width  = pgm_read_byte(&Logo[image].width);
    byte height = pgm_read_byte(&Logo[image].height);
    const byte *imgPtr = reinterpret_cast<const byte*>(pgm_read_ptr(&Logo[image].data));
    oledLeft.clear();
    oledRight.clear();
    int  a = 0;
    // Display the image
    for (byte b=0; b < height/8; b++) {
        oledLeft.setCursor(xPos,  yPos + b);
        oledRight.setCursor(xPos, yPos + b);
        for (byte i=0; i<width; i++) {
            oledLeft.ssd1306WriteRam(pgm_read_byte(&imgPtr[a]));
            oledRight.ssd1306WriteRam(pgm_read_byte(&imgPtr[a+width-1-i*2]));
            a++;
        }
    }
} /**/
