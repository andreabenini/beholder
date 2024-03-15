## Partition table
Getting current partition table from the device
```sh
idf.py partition-table
# *******************************************************************************
## ESP-IDF Partition Table for this project
## Name,     Type,  SubType, Offset,   Size,  Flags
#  nvs,      data,  nvs,     0x9000,   16K,
#  otadata,  data,  ota,     0xd000,   8K,
#  phy_init, data,  phy,     0xf000,   4K,
#  factory,  app,   factory, 0x10000,  200K,
#  ota_0,    app,   ota_0,   0x50000,  1400K,
#  ota_1,    app,   ota_1,   0x1b0000, 1400K,
# *******************************************************************************
```

## Flashing device
```sh
# Default command applied from ESP-IDF Visual Studio Code plugin
/home/ben/.espressif/python_env/idf5.3_py3.11_env/bin/python
    /home/ben/esp/esp-idf/components/esptool_py/esptool/esptool.py
    -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset 
    --chip esp32 write_flash --flash_mode dio --flash_freq 40m
    --flash_size 4MB 
    0x1000 bootloader/bootloader.bin
    0x10000 server.bin
    0x8000 partition_table/partition-table.bin
    0xd000 ota_data_initial.bin

# Flashing a single partition, simplest command ever. Always check partition start address
python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash 0x110000 ./build/server.bin
```

### Flashing software (on partition OTA_0 only)
```sh
python $IDF_PATH/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 460800 --chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x50000 ./image/partition.ota0.bin
```
