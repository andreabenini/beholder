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
