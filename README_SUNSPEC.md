# SunSpec Integration for Sol-Ark Inverter

This document describes the SunSpec compliance implementation for the Sol-Ark inverter in the EMS-Dev platform.

## Overview

This implementation allows the EMS-Dev platform to act as a SunSpec-compliant Modbus RTU server, exposing Sol-Ark inverter data in a standardized format that can be read by any SunSpec-compatible client.

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

The SunSpec-compliant Modbus server runs on the second RS-485 port (RS485_RX_2, RS485_TX_2) with the following settings:
- Modbus address: 1
- Baud rate: 9600
- Data format: 8N1 (8 data bits, no parity, 1 stop bit)

Any SunSpec-compatible Modbus client can connect to this server to read the standardized inverter data.

## Testing

You can test the SunSpec implementation using:
1. SunSpec-compatible client software (e.g., SunSpec Dashboard)
2. Modbus polling tools with the appropriate register map
3. pysunspec2 library for Python-based testing

## References

- SunSpec Alliance: https://sunspec.org/
- pysunspec2 library: https://github.com/sunspec/pysunspec2
- SunSpec Model 701: https://github.com/sunspec/models/tree/dde49cc598def3f179c8db0ec15d4281cb0af3aa
