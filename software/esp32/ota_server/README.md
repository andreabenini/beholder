# Sample OTA Server for providing firmware updates to ESP-CAM devices
## Requirements
- python3
- flask (better inside a VirtualEnv)

## Setup
- Create a Virtual Environment for this python server
- Update `pip` and install `flask` as a dependency
- Create a softlink named `firmware.bin` to the ESP binary file, something like:
    ```sh
    # create a link in the same directory where python 'ota_server' is
    ln -s /path/where/esp/file/is/master.bin  firmware.bin
    ```
### Environment
```sh
# Enter the environment
source bin/activate

# Start the server
./ota_server
# ==============================================
# Basic OTA server
#     Listening on: http://localhost:8000 
# ==============================================
# 
#      /update      -> Download firmware.bin
#      /firmware    -> Download firmware.bin
```
