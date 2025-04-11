EMS Dev Kit

ESP32 peripheral module Arduino framework examples:
 1. RS-485 (MODBUS RTU)
 2. Buttons (voltage divider array on analog GPIO)

Installation:
 1. install VSCode and PlatformIO VSCode extension
 2. clone this repository
 3. open the repository folder in vscode as a PlatformIO project (PlatformIO->Open Project)
 4. allow dependencies to download
 5. build and flash

Port Labs April 22-23 workshop code challenges 
A/ meaningful to behind-the-meter Sunspec self cert readyness of modbus/canbus IWF hub:
1. Sunspec BESS Modbus register support
2. Sunspec Inverter Modbus register support
3. Sunspec Solar Controller Modbus register support

B/ meaningful to front-of-meter IEEE ISV StreetPoleEMS:
1. OPENPLC subset libs integration - specifically of interest are 2 functions
 i) IFTTT Rules engine - option to pass thru rules filter during 
   a) OPENAMI periodic publishes, 
   b) when subscribed  command and control  is received from wan
   c) local event threshold is triggered
 ii) support for legacy modbus PLC endpoint nodes
2. 18650 optional holdover battery backup of EMS - store restorable "warm" timestamped states/events  into flash
3. RTC clock - sleep and deep sleep of peripherals and/or ESP32S3
4. BLE mesh as LAN
5. LR BLE mesh as WAN
6. LORA Meshtastic as LAN
7. LORA Meshtastic as WAN
8. G3 RF+PLC MAC/PHY module support as WAN/LAN mesh networking
9. ENACCESS OPEN SMART METER subset libs integration for support of Paygo Dongle prepaid and postpaid meter
10. Smartphone IT administrator mated app - secure authenticated BLE OA&M and OTA API


