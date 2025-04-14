# EIOT.Energy EMS Dev Kit

## Overview
A development kit based on the ESP32S3 N16R8 DEV KIT C1 for energy management systems (EMS) with support for various communication protocols and peripherals.

## Features
This development kit supports multiple peripherals using the Arduino framework:
- RS-485 MODBUS RTU communication
- CANBUS V2.0 interface
- Input buttons (using voltage divider array on analog GPIO)

## Installation Guide
### Prerequisites

- A computer with internet connection
- ESP32S3 N16R8 DEV KIT C1 hardware
- USB-C cable for connecting the development board to your computer

### Step 1: Install Visual Studio Code
Download Visual Studio Code from https://code.visualstudio.com/
Follow the installation instructions for your operating system:

**Windows:** Run the installer and follow the prompts
**macOS:** Drag the application to your Applications folder

### Step 2: Install PlatformIO Extension

Open VSCode
Click on the Extensions icon in the left sidebar (or press Ctrl+Shift+X)
Search for "PlatformIO IDE"
Click "Install" on the PlatformIO IDE extension
Wait for the installation to complete (this may take a few minutes)
Restart VSCode when prompted

### Step 3: Clone the Repository
Open a terminal/command prompt
Navigate to the directory where you want to store the project
Clone the repository using git:
git clone https://github.com/yourusername/EIOT.Energy-EMS-Dev-Kit.git
(Replace the URL with the actual repository URL)

### Step 4: Open the Project in VSCode

In VSCode, click on the PlatformIO icon in the left sidebar
Select "Open Project" from the PlatformIO home screen
Navigate to the cloned repository folder and select it
Wait for VSCode to load the project and initialize PlatformIO

### Step 5: Configure the Project

Wait for PlatformIO to download all required dependencies (libraries)
**IMPORTANT:** Set the environment to ESP32S3 N16R8 DEV KIT C
Open the platformio.ini file in the project root
Make sure the environment section contains [env:esp32s3_n16r8] or similar
If not, add or modify the environment section to match the ESP32S3 N16R8 DEV KIT C



### Step 6: Build and Flash the Firmware
Connect your ESP32S3 DEV KIT to your computer via USB
In VSCode, click on the PlatformIO icon in the left sidebar
Select "Project Tasks" from the menu
Under "General", click "Build" to compile the project
After successful build, click "Upload" to flash the firmware to your device
Monitor the progress in the terminal window at the bottom of VSCode

### Troubleshooting

If you encounter upload errors, ensure that:
The correct USB port is selected (can be changed in platformio.ini)
You have proper USB drivers installed for your development board
Your board is in bootloader mode (if required)

For dependency issues, try running "Clean" before "Build"
Check the PlatformIO documentation for additional help: https://docs.platformio.org/

## Development Tracks

### Lane A: Behind-the-Meter Sunspec Integration
These challenges focus on Sunspec self-certification readiness for Modbus/Canbus IWF hubcore functions:

#### Sunspec Protocol Support
- Sunspec BESS Gridtie Battery Modbus register support
- Sunspec Gridtie Inverter Modbus register support
- Sunspec Gridtie Solar Controller Modbus register support

#### Integration Plugins
- Sol-Ark Modbus LAN side R/W transcoder (converts to Sunspec Inverter Modbus formats)
- Custom BMS, Inverter, Solar/Wind controller Modbus RTU plugins
- Behind-the-Meter OPENAMI bidirectional monitor and control PUB/SUB framework
- "Plug-in Solar" device plugin
- AC Energy Meter plugins (Single Phase, Split Phase, Three Phase)
- VFD Modbus RTU plugin
- Energy IoT device plugin
- Z-wave Plus IoT device interoperability plugin

### Lane B: Front-of-Meter IEEE ISV StreetPoleEMS Integrations

#### Framework & Networking
- Front-of-Meter OPENAMI bidirectional monitor and control PUB/SUB framework
- StreetPoleEMS MESH distributed intelligence Pub/Sub networking
- FLEXMEASURES layering export for ESP32S3
- EMS MESH networking to N:1 StreetPole Linux Aggregator with distributed AI Energy Policy

#### Metering Plugins
- IVY Metering Bidirectional AC/DC powerflow RCD and RVD detection
- Donsun DLMS/STS prepaid meter integration
- Donsun postpaid meter integration
- IVY Metering AC/DC meter prepaid/postpaid integration (DLMS/STS)
- AC Energy Meter plugins (Single Phase, Split Phase, Three Phase)
- Bidirectional AC/DC powerflow RCD and RVD leakage detection
- EVSE AC/DC Charge/Discharge controller plugins
- VFD Modbus RTU plugin
- Energy IoT device plugin

#### Additional Integrations
- ENACCESS OPEN SMART METER libraries for Paygo Dongle support
- OPENPLC integration with IFTTT Rules engine and Modbus PLC endpoint support
- 18650 battery backup with state persistence to flash memory
- RTC clock with sleep and deep sleep power management

### Extended Networking Capabilities
- Ethernet MAC/PHY WAN/LAN dual port
- BLE mesh LAN networking
- G3 ALLIANCE RF+PLC MAC/PHY module for WAN/LAN mesh networking
- LR BLE mesh WAN networking
- LORA Meshtastic LAN/WAN networking
- LORAWAN WAN connectivity
- LTE CATM1 global SIM/eSIM radio module
- Starlink MAC/PHY WWAN radio module integration

## Port Labs Workshop (April 22-23)
The code challenges listed above will be covered during the Port Labs workshop. Participants can choose challenges from either Lane A or Lane B based on their interests and requirements.

## Contributing
Feel free to suggest additional integration ideas or contribute to existing challenges.
