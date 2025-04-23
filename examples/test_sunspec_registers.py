#!/usr/bin/env python3
"""
SunSpec Register Test Script

This script tests the SunSpec implementation by directly reading Modbus registers
from the EMS-Dev platform. It doesn't require the pysunspec2 library, only pymodbus.

Requirements:
- Python 3.6+
- pymodbus (pip install pymodbus)
- pyserial (pip install pyserial)

Usage:
python test_sunspec_registers.py [serial_port] [modbus_address]

Example:
python test_sunspec_registers.py /dev/ttyUSB0 1
"""

import sys
import time
from pymodbus.client.sync import ModbusSerialClient
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder

def read_sunspec_string(client, address, slave_id, length=8):
    """Read a SunSpec string from Modbus registers."""
    response = client.read_holding_registers(address, length, slave=slave_id)
    if not response.isError():
        decoder = BinaryPayloadDecoder.fromRegisters(
            response.registers,
            byteorder=Endian.Big,
            wordorder=Endian.Big
        )
        # SunSpec strings are stored as UTF-8 in big-endian registers
        string_bytes = decoder.decode_string(length * 2)
        return string_bytes.decode('utf-8').rstrip('\x00')
    return None

def main():
    # Parse command line arguments
    if len(sys.argv) < 2:
        print("Usage: python test_sunspec_registers.py [serial_port] [modbus_address]")
        print("Example: python test_sunspec_registers.py /dev/ttyUSB0 1")
        sys.exit(1)
    
    serial_port = sys.argv[1]
    slave_id = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    
    # Create Modbus client
    client = ModbusSerialClient(
        method='rtu',
        port=serial_port,
        baudrate=9600,
        bytesize=8,
        parity='N',
        stopbits=1,
        timeout=1
    )
    
    try:
        # Connect to the Modbus device
        if not client.connect():
            print("Failed to connect to the Modbus device")
            sys.exit(1)
        
        print(f"Connected to Modbus device at {serial_port}, address {slave_id}")
        
        # Test 1: Check for SunSpec identifier "SunS"
        print("\nTest 1: Checking for SunSpec identifier...")
        response = client.read_holding_registers(0, 2, slave=slave_id)
        if not response.isError():
            sunspec_id = (response.registers[0] << 16) | response.registers[1]
            print(f"SunSpec ID: 0x{sunspec_id:08X}")
            
            # Check if the ID matches "SunS" (0x53756e53)
            if sunspec_id == 0x53756e53:
                print("✓ SunSpec identifier found!")
            else:
                print("✗ SunSpec identifier not found or incorrect")
        else:
            print(f"✗ Failed to read SunSpec identifier: {response}")
        
        # Test 2: Check Common Model (1)
        print("\nTest 2: Checking Common Model (1)...")
        response = client.read_holding_registers(2, 2, slave=slave_id)
        if not response.isError():
            model_id = response.registers[0]
            model_length = response.registers[1]
            print(f"Model ID: {model_id}")
            print(f"Model Length: {model_length}")
            
            if model_id == 1:
                print("✓ Common Model (1) found!")
                
                # Read manufacturer
                manufacturer = read_sunspec_string(client, 2 + 4, slave_id)
                model = read_sunspec_string(client, 2 + 20, slave_id)
                options = read_sunspec_string(client, 2 + 36, slave_id)
                version = read_sunspec_string(client, 2 + 52, slave_id)
                serial = read_sunspec_string(client, 2 + 54, slave_id)
                
                print(f"Manufacturer: {manufacturer}")
                print(f"Model: {model}")
                print(f"Options: {options}")
                print(f"Version: {version}")
                print(f"Serial Number: {serial}")
            else:
                print("✗ Common Model (1) not found or incorrect")
        else:
            print(f"✗ Failed to read Common Model: {response}")
        
        # Test 3: Check Inverter Model (701)
        print("\nTest 3: Checking Inverter Model (701)...")
        # Calculate the start address of the inverter model (after common model)
        inverter_start = 2 + 65 + 2  # Base + Common model length + 2 for next model header
        
        response = client.read_holding_registers(inverter_start, 2, slave=slave_id)
        if not response.isError():
            model_id = response.registers[0]
            model_length = response.registers[1]
            print(f"Model ID: {model_id}")
            print(f"Model Length: {model_length}")
            
            if model_id == 701:
                print("✓ Inverter Model (701) found!")
                
                # Read some key inverter data points
                # AC measurements
                response = client.read_holding_registers(inverter_start + 4, 2, slave=slave_id)
                if not response.isError():
                    ac_current = response.registers[0]
                    ac_current_sf = response.registers[1]
                    ac_current_value = ac_current * (10 ** ac_current_sf)
                    print(f"AC Current: {ac_current_value} A")
                
                response = client.read_holding_registers(inverter_start + 12, 2, slave=slave_id)
                if not response.isError():
                    ac_voltage = response.registers[0]
                    ac_voltage_sf = response.registers[1]
                    ac_voltage_value = ac_voltage * (10 ** ac_voltage_sf)
                    print(f"AC Voltage: {ac_voltage_value} V")
                
                response = client.read_holding_registers(inverter_start + 24, 2, slave=slave_id)
                if not response.isError():
                    ac_power = response.registers[0]
                    ac_power_sf = response.registers[1]
                    ac_power_value = ac_power * (10 ** ac_power_sf)
                    print(f"AC Power: {ac_power_value} W")
                
                # DC measurements
                response = client.read_holding_registers(inverter_start + 36, 2, slave=slave_id)
                if not response.isError():
                    dc_current = response.registers[0]
                    dc_current_sf = response.registers[1]
                    dc_current_value = dc_current * (10 ** dc_current_sf)
                    print(f"DC Current: {dc_current_value} A")
                
                response = client.read_holding_registers(inverter_start + 38, 2, slave=slave_id)
                if not response.isError():
                    dc_voltage = response.registers[0]
                    dc_voltage_sf = response.registers[1]
                    dc_voltage_value = dc_voltage * (10 ** dc_voltage_sf)
                    print(f"DC Voltage: {dc_voltage_value} V")
                
                response = client.read_holding_registers(inverter_start + 40, 2, slave=slave_id)
                if not response.isError():
                    dc_power = response.registers[0]
                    dc_power_sf = response.registers[1]
                    dc_power_value = dc_power * (10 ** dc_power_sf)
                    print(f"DC Power: {dc_power_value} W")
                
                # Status
                response = client.read_holding_registers(inverter_start + 50, 2, slave=slave_id)
                if not response.isError():
                    status = response.registers[0]
                    vendor_status = response.registers[1]
                    print(f"Inverter Status: 0x{status:04X}")
                    print(f"Vendor Status: 0x{vendor_status:04X}")
                    
                    # Decode vendor status
                    print("Vendor Status Flags:")
                    print(f"  Grid Connected: {'Yes' if vendor_status & 0x0001 else 'No'}")
                    print(f"  Generator Connected: {'Yes' if vendor_status & 0x0002 else 'No'}")
                    print(f"  Battery Charging: {'Yes' if vendor_status & 0x0004 else 'No'}")
                    print(f"  Battery Discharging: {'Yes' if vendor_status & 0x0008 else 'No'}")
                    print(f"  Selling to Grid: {'Yes' if vendor_status & 0x0010 else 'No'}")
                    print(f"  Buying from Grid: {'Yes' if vendor_status & 0x0020 else 'No'}")
            else:
                print("✗ Inverter Model (701) not found or incorrect")
        else:
            print(f"✗ Failed to read Inverter Model: {response}")
        
        print("\nSunSpec register test completed.")
    
    except Exception as e:
        print(f"Error: {e}")
    
    finally:
        client.close()
        print("Connection closed.")

if __name__ == "__main__":
    main()
