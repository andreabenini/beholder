| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# Bluetooth joypad controller sample software

## Folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

## sdkconfig
- `sdkconfig` file will be automatically generated with `idf.py set-target esp32` command and it's based on architecture feature
    related configuration `sdkconfig.defaults.*` files

## bluetooth
gamepad information
```txt
    BT : a0:5a:5e:11:ac:ff, COD: major: PERIPHERAL, minor: 2, service: 0x001, RSSI: -70, NAME: Wireless Controller
                            RSSI: -70, USAGE: GENERIC, COD: PERIPHERAL[GAMEPAD] srv 0x001, UUID16: 0x0000, NAME: Wireless Controller
```
