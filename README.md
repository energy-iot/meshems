EIOT.Energy EMS Dev Kit - ESP32S3 N16R8 DEV KIT C1

ESP32S3 peripheral module Arduino framework examples:
 1. RS-485 MODBUS RTU
 2. CANBUS V2.0
 3. Buttons (voltage divider array on analog GPIO)

Installation:
 1. install VSCode and PlatformIO VSCode extension
 2. clone this repository
 3. open the repository folder in vscode as a PlatformIO project (PlatformIO->Open Project)
 4. allow dependencies to download - SET ENVIRONMENT TO ESP32S3 N16R8 DEV KIT C
 5. build and flash

Port Labs April 22-23 workshop code challenges:  
LANE A/ meaningful to behind-the-meter Sunspec self-cert readiness of Modbus/Canbus IWF hubcore functions:
1. Sunspec BESS Gridtie Battery Modbus register support
2. Sunspec Gridtie Inverter Modbus register support
3. Sunspec Gridtie Solar Controller Modbus register support
4. Sol-Ark Modbus lan side R/W transcoder plugin to Sunspec Inverter Modbus formats - subset iterations testable to Sunspec self cert tool
5. BYO favorite BMS, Inverter, Solar/Wind controller modbus RTU plugin
BUILDING STU/MTU addons
6. BehindTheMeter OPENAMI 2way monitor and control PUB/SUB topic structure framework  
7. BYO "PLUG-IN SOLAR" DEVICE plugin
8. BYO SINGLE PHASE AC Energy Meter plugin
9. BYO SPLIT PHASE AC Energy Meter plugin
10. BYO THREE PHASE AC Energy Meter plugin
11. BYO VFD Modbus RTU plugin
12. BYO Energy Iot thing plugin
13. BYO Z-wave Plus Iot device interop plugin
14. Suggestions?  Other Building Iot Systems interworking endpoint devices & Hubs

LANE B/ Front-of-meter IEEE ISV StreetPoleEMS meaningful integrations:
1. FrontOfTheMeter OPENAMI 2way monitor and control PUB/SUB topic structure framework
2. STREETPOLEEMS MESH distributed Intelligence Pub/Sub intra-networking
3. FLEXMEASURES layering export into ESP32S3
4. EMS MESH networking to N:1 StreetPole Linux Aggregator distributed AI Energy Policy Decision Point (OPENEMS Edge, Pandas timeseries Analytics, EnergyNet 
5. IVY Metering Bidirectional AC/DC powerflow RCD and RVD detection device plugin
6. Donsun DLMS/STS prepaid meter plugin iterations
3. Donsun postpaid meter plugin iterations
6. IVY Metering AC/DC meter prepaid/postpaid plugin iterations (DLMS/STS)
7. BYO SINGLE PHASE AC Energy Meter plugin
8. BYO SPLIT PHASE AC Energy Meter plugin
9. BYO THREE PHASE AC Energy Meter plugin
8. BYO Bidirectional AC/DC powerflow RCD and RVD leakage detection device plugin
10. BYO EVSE PHASE AC Charge/Discharge controller plugin (ETEK Electric EKEPC2) 
11. BYO EVSE PHASE DC Charge/Discharge controller plugin
12. BYO VFD Modbus RTU plugin
13. BYO Energy Iot thing plugin
14. ENACCESS OPEN SMART METER subset libs integration for support of Paygo Dongle prepaid and postpaid meter
15. OPENPLC subset libs integration - specifically of interest are 2 functions
 i) IFTTT Rules engine - option to pass thru rules filter during 
   a) OPENAMI periodic publishes, 
   b) when subscribed  command and control  is received from wan
   c) local event threshold is triggered
 ii) support for legacy Modbus PLC endpoint nodes
16. 18650 optional holdover battery backup of EMS - store restorable "warm" timestamped states/events  into flash
17. RTC clock - sleep and deep sleep of peripherals and/or ESP32S3
    
Networking LAN/WAN extended radio module WWAN/WAN/LAN:
19. ETHERNET MAC/PHY wan/lan dual port 
20. BLE mesh as LAN
21. G3 ALLIANCE RF+PLC MAC/PHY module support as WAN/LAN mesh networking
22. LR BLE mesh as WAN
23. LORA Meshtastic as LAN
24. LORA Meshtastic as WAN
25. LORAWAN as WAN
26. LTE CATM1 global SIM/eSIM MAC/PHY radio module
27. Starlink MAC/PHY WWAN radio module/device 


    Suggestions?  Other Building Iot Systems interworking endpoint devices & Hubs
    Smartphone IT administrator mated app - secure authenticated on-demand BLE/WiFi OA&M and OTA API


