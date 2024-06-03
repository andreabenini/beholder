# bluepad32 library

## Installation
**NOTE:** Execute all following steps in a IDF enabled shell, 
  see [Install](install.md) notes for it
```sh
# Clone repository
cd
cd esp
git clone https://github.com/bluekitchen/btstack
# git clone --recursive https://github.com/ricardoquesada/bluepad32.git
# # Apply provided patches
# cd bluepad32/external/btstack/
# git apply ../patches/*.patch
# Integrate it to ESP-IDF
cd port/esp32               # ~/esp/bluepad32/external/btstack/port/esp32
# - Double check $IDF_PATH environment variable, it must exists before proceeding
# - scripts always takes care about the ./components path and the entire ESP-IDF env.
#   It can also deal with updates too
./integrate_btstack.py
```

## Examples
- Located in bluepad32/examples/esp32
- `idf.py clean`
- `idf.py set-target esp32c3`
- `idf.py build`
- `idf.py -p /dev/ttyACM0 flash`
- `idf.py -p /dev/ttyACM0 monitor`

Configuration
- `idf.py -p /dev/ttyACM0 monitor`


&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>

# Useful projects and links
- https://github.com/jvpernis/esp32-ps3 (not supporting latest ESP-IDF framework)
- https://github.com/openobjects/PS4-esp32 (a port of the above, somewhat workable but outdated too)
- https://www.google.com/search?q=esp32c3+ps3+controller&newwindow=1&sca_esv=359d6b2fe139ffab&ei=zNE_ZtmsNraC9u8PtcC5oAo&oq=esp32c3+ps3&gs_lp=Egxnd3Mtd2l6LXNlcnAiC2VzcDMyYzMgcHMzKgIIADIFECEYoAEyBRAhGKABMgUQIRigATIFECEYoAFI33pQAFiPWXAAeACQAQCYAYQBoAGJB6oBAzkuMbgBA8gBAPgBAZgCCqACrwfCAhcQLhiABBiRAhixAxjRAxiDARjHARiKBcICFBAuGIAEGJECGLEDGNEDGMcBGIoFwgIREC4YgAQYsQMY0QMYgwEYxwHCAgsQABiABBixAxiDAcICCxAuGIAEGLEDGIMBwgIFEAAYgATCAg4QABiABBixAxiDARiKBcICDhAuGIAEGLEDGNEDGMcBwgIUEC4YgAQYsQMY0QMYgwEYxwEYigXCAiYQLhiABBiRAhixAxjRAxiDARjHARiKBRiXBRjcBBjeBBjgBNgBAcICCxAAGIAEGJECGIoFwgIKEAAYgAQYQxiKBcICDRAuGIAEGEMY5QQYigXCAg0QLhiABBixAxhDGIoFwgIIEAAYgAQYsQPCAg0QABiABBixAxhDGIoFwgIOEAAYgAQYkQIYsQMYigXCAggQLhiABBjlBMICBhAAGBYYHsICCBAAGIAEGKIEmAMAugYGCAEQARgUkgcDNi40oAfUUg&sclient=gws-wiz-serp#ip=1
- https://github.com/ricardoquesada/bluepad32 (latest kid in town, based on btstack codebase)
- https://github.com/bluekitchen/btstack
- https://github.com/bluekitchen/btstack/tree/master/port/esp32
- https://bluekitchen-gmbh.com/btstack/#examples/examples/
- https://github.com/ricardoquesada/bluepad32/issues/65#issuecomment-1987046804
- https://github.com/ricardoquesada/bluepad32/issues?page=1&q=is%3Aissue
- https://bluepad32.readthedocs.io/en/stable/supported_gamepads/
- https://github.com/h2zero/esp-nimble-cpp
