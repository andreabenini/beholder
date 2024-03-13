# Installation
## ESP-IDF
No visual studio code plugins are stricly required, you can use the whole environment from command line:
```sh
# Install ESP-IDF
mkdir $HOME/esp
cd esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Install ESP32 Camera libraries
cd components
git clone https://github.com/espressif/esp32-camera
cd ..

# Add user to 'uucp' in /etc/group

# Install ESP-IDF
./install.sh all
```

### **Enter virtual environment**
```sh
# . ./export.sh
. $HOME/esp/esp-idf/export.sh
idf.py --help
# Examples in:  $HOME/esp/esp-idf/examples
```

### **Create and build a sample project**
```sh
cd ~/esp
cp -r $IDF_PATH/examples/get-started/hello_world .
cd hello_world
idf.py build
```

## Visual Studio Plugin: "Espressif IDF"
- Install the visual studio plugin from **Extensions**, it's called "Espressif IDF"
- From _Command Palette_ (Ctrl+Shift+P) select: **"ESP-IDF: Configure ESP-IDF extension"**
- To configure this extension there are two options: Express, Advanced. Select _"Express"_
- From configuration window set these values in fields:
  - Select download server: **Github**
  - Uncheck flag **"Show all ESP-IDF tags"**
  - Select ESP-IDF version: **Find ESP-IDF in your system**
  - Enter ESP-IDF directory (IDF_PATH): **/home/ben/esp/esp-idf**
  - Enter ESP-IDF Tools directory (IDF_TOOLS_PATH): **/home/ben/esp/esp-idf**
  - Select Python version: **/usr/bin/python**
- Close then window when all settings have been configured


## Project Setup
For every project, by default, IntelliSense is present, but the environment may not give
any indication if there is a problem. To fix that:
- Go to `.vscode` > `settings.json` and set the `C_Cpp.intelliSenseEngine` to `default`.  
  Previously it was set to `Tag Parser.`
- Now if there is an error in a C file, a red wavy line appears beneath that code


## New Project
To create a new ESP-IDF project just:
- Select: **"ESP-IDF: New Project"** from _"Command Palette" (Ctrl+Shift+P)_
- From 'New Project' form, first page:
  - Enter the project name
  - Enter the project directory
  - Choose ESP-IDF board, pick _'Custom Board'_ if unsure
  - if _Custom Board_ is selected the additional combo box: "Choose ESP-IDF Target"
    must be set to: "ESP32 module"
  - Select the dedicated serial port if applicable or let it unconfigured if unsure
  - Press "Choose Template" when finished
- From second page:
  - Set the combo box to "ESP-IDF", original value is: "Extension"
  - Select project skeleton: "`/ esp-idf / get-started / sample_project`"
  - Press button "Create project using template sample_project"
- A new project will be created
- Go to `.vscode` > `settings.json` and set the `C_Cpp.intelliSenseEngine` to `default`.  
  Previously it was set to `Tag Parser`. Now if there is an error in a C file, a red wavy
  line appears beneath that code
- Press button "ESP-IDF Build project" _(Ctrl+E, B)_ in the bottom bar to create the initial build

---