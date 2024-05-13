# PS3 SixAxis DualShock BT Controller
This amazing electronic device is cheap and precise enough to be used as a primary 
input device for driving the robot when paired with a human.
I have decided to enable builtin bluetooth capabilities in order to support direct
connection with it, this directory contains utilities and documentation for a full
PS3 controller BT support.


## Sixpair utility
Compile and use it in order to attach your controller to the robot, address reported
in the PS3 controller should be the same of the bot firmware, for example:
```sh
./sixpair 1a:2b:3c:01:01:01
```
#### Compile
- This program uses dynamic usb libraries, usually they need to be installed and might
    have these names: `libusb-1_0-devel`, `libusb-compat-devel`.  
- `libusb-compat-devel` provides compatibility level with older linux bt stack somewhat
    mandatory to use it with the program
- Compile with: `gcc sixpair.c -lusb -o sixpair` or use `make.sh` provided utility


## Arduino demo
- Open Arduino IDE
- Menu File/Preferences, field: _"Additional Boards Manager URLs"_, add this board info:
    > https://dl.espressif.com/dl/package_esp32_index.json
- Press **OK** to confirm
- Menu _Tools/Boards/Board Manager_, search _"esp32"_ and select proper board: esp32-c3 or whatever
- Press **Install** button and then close the board manager with the **Close** button
- Menu _Tools/Manage Libraries_, search: _"ps3 controller host"_ and select the
    "Jeffrey van Pernis" library, press **Install** button and **Close** once done
- Open _"arduino_ps3/arduino_ps3.ino"_ sketch with Arduino IDE, verify, compile and upload as usual
    > #### arduino_ps3.ino
    > Arduino based test utility for dealing with ps3 controllers.
    > This sample uses the arduino IDE and provide a working demo for testing
    > controller pairing and basic events from keys and thumbsticks.

