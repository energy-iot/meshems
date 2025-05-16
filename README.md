# SunSpec Integration for Sol-Ark Inverter

This document describes the SunSpec compliance implementation for the Sol-Ark inverter in the EMS-Dev platform.

## Overview

This implementation allows the EMS-Dev platform to act as a SunSpec-compliant Modbus TCP/IP server, exposing Sol-Ark inverter data in a standardized format that can be read by any SunSpec-compatible client.

The implementation follows the SunSpec Alliance specifications and includes:
- SunSpec Common Model (1) - Basic device information
- SunSpec Inverter Model (701) - Inverter data

## Architecture

The implementation consists of the following components:

1. **SunSpec Models Definition** (`include/sunspec_models.h`): Defines the SunSpec model structure, register maps, and constants.

2. **SunSpec Mapper** (`src/sunspec_mapper.cpp`): Implements the mapping between Sol-Ark data and SunSpec registers.

3. **Modbus Client Integration** (`src/modbus_client.cpp`): Initializes the SunSpec models and updates the registers with Sol-Ark data.

## SunSpec Model Implementation

### Common Model (1)

The Common Model provides basic information about the device:
- Manufacturer: "Sol-Ark"
- Model: "SolArk-LV"
- Options: "EMS-Dev"
- Version: "1.0"
- Serial Number: "DEMO-1"

### Inverter Model (701)

The Inverter Model provides real-time data from the Sol-Ark inverter:
- AC measurements (current, voltage, power, frequency, energy)
- DC measurements (current, voltage, power)
- Temperature
- Status information

## Register Map

The SunSpec register map is implemented in the Modbus holding registers:

- Base address: 40000
- SunSpec ID marker ("SunS"): Registers 40000-40001
- Common Model (1): Starts at register 40002
- Inverter Model (701): Starts at register 40069

## Data Mapping

The implementation maps Sol-Ark data to SunSpec registers as follows:

### AC Measurements
- AC current: Average of inverter L1 and L2 currents
- AC voltage: Inverter voltage
- AC power: Inverter output power
- AC frequency: Inverter frequency
- AC energy: Load energy (converted from kWh to Wh)

### DC Measurements
- DC current: Battery current
- DC voltage: Battery voltage
- DC power: Battery power

### Temperature
- Cabinet temperature: Battery temperature

### Status
- Inverter status: Derived from inverter power
- Vendor-specific status: Grid connection, generator connection, battery charging/discharging, grid selling/buying

## Usage

The SunSpec-compliant Modbus TCP/IP server runs with the following settings:
- WiFi: Connects to an existing WiFi network
- IP Address: Dynamically assigned by DHCP (displayed on the OLED screen)
- TCP Port: 8502
- Protocol: Modbus TCP/IP

Any SunSpec-compatible Modbus TCP client can connect to this server to read the standardized inverter data.

## Testing

You can test the SunSpec implementation using:
1. SunSpec-compatible client software (e.g., SunSpec Dashboard)
2. Modbus TCP polling tools with the appropriate register map (e.g., ModbusPoll, QModMaster)
3. pysunspec2 library for Python-based testing (with TCP transport)
4. Example Python script in examples/sunspec_client_example.py (modified for TCP)

## Example TCP Client Connection

Using the pysunspec2 library:

```python
import sunspec2.modbus.client as client

# Connect to the SunSpec TCP server
c = client.SunSpecModbusClientTCP(host='192.168.1.x', port=8502)
c.connect()

# Read the models
c.scan()

# Access the inverter model
inv = c.models[701]

# Read inverter data
print(f"AC Power: {inv.points['W'].value} W")
print(f"AC Voltage: {inv.points['PhVphA'].value} V")
print(f"AC Frequency: {inv.points['Hz'].value} Hz")
print(f"DC Voltage: {inv.points['DCV'].value} V")
print(f"DC Current: {inv.points['DCA'].value} A")
print(f"Cabinet Temperature: {inv.points['TmpCab'].value} °C")

# Close the connection
c.close()
```

## References

- SunSpec Alliance: https://sunspec.org/
- pysunspec2 library: https://github.com/sunspec/pysunspec2
- SunSpec Model 701: https://github.com/sunspec/models/tree/dde49cc598def3f179c8db0ec15d4281cb0af3aa
- Modbus-ESP8266 library: https://github.com/emelianov/modbus-esp8266
