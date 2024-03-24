#!/usr/bin/env bash
#
# Start a serial console for some debugging on secondary serial port
#
/home/ben/.espressif/python_env/idf5.3_py3.11_env/bin/python /home/ben/esp/esp-idf/tools/idf_monitor.py -p /dev/ttyUSBcom -b 9600
