#!/usr/bin/env python3
"""
Debug SunSpec Register Layout

This script reads and displays the actual register values to debug the SunSpec layout.
"""

import sys
from pathlib import Path

# Add parent directory to path to import ems module
sys.path.insert(0, str(Path(__file__).parent.parent))

from pymodbus.client import ModbusTcpClient


def debug_registers(host="localhost", port=8502, unit_id=1):
    """Debug register layout"""
    
    try:
        client = ModbusTcpClient(host=host, port=port, timeout=5)
        
        if not client.connect():
            print("Failed to connect to Modbus TCP server")
            return
        
        print("Connected to Modbus TCP server")
        print("="*60)
        
        # Read SunSpec header
        print("SunSpec Header (registers 0-1):")
        result = client.read_holding_registers(address=0, count=2, slave=unit_id)
        if not result.isError():
            sunspec_id = (result.registers[0] << 16) | result.registers[1]
            print(f"  Registers 0-1: {result.registers}")
            print(f"  SunSpec ID: 0x{sunspec_id:08X} ({'SunS' if sunspec_id == 0x53756E53 else 'Not SunSpec'})")
        else:
            print(f"  Error: {result}")
        
        # Read Common Model header
        print("\nCommon Model Header (registers 2-3):")
        result = client.read_holding_registers(address=2, count=2, slave=unit_id)
        if not result.isError():
            print(f"  Registers 2-3: {result.registers}")
            print(f"  Model ID: {result.registers[0]}, Length: {result.registers[1]}")
        else:
            print(f"  Error: {result}")
        
        # Read some Common Model data
        print("\nCommon Model Data (registers 4-19 - Manufacturer):")
        result = client.read_holding_registers(address=4, count=16, slave=unit_id)
        if not result.isError():
            print(f"  Registers 4-19: {result.registers}")
            # Convert to string
            manufacturer = ""
            for reg in result.registers:
                char1 = (reg >> 8) & 0xFF
                char2 = reg & 0xFF
                if char1 != 0:
                    manufacturer += chr(char1)
                if char2 != 0:
                    manufacturer += chr(char2)
            print(f"  Manufacturer: '{manufacturer.strip()}'")
        else:
            print(f"  Error: {result}")
        
        # Check where the next model should be
        print("\nInverter Model Location (registers 100-101):")
        result = client.read_holding_registers(address=100, count=2, slave=unit_id)
        if not result.isError():
            print(f"  Registers 100-101: {result.registers}")
            print(f"  Model ID: {result.registers[0]}, Length: {result.registers[1]}")
        else:
            print(f"  Error: {result}")
        
        # Check battery model location
        print("\nBattery Model Location (registers 200-201):")
        result = client.read_holding_registers(address=200, count=2, slave=unit_id)
        if not result.isError():
            print(f"  Registers 200-201: {result.registers}")
            print(f"  Model ID: {result.registers[0]}, Length: {result.registers[1]}")
        else:
            print(f"  Error: {result}")
        
        # Check end marker location
        print("\nEnd Marker Location (registers 250-251):")
        result = client.read_holding_registers(address=250, count=2, slave=unit_id)
        if not result.isError():
            print(f"  Registers 250-251: {result.registers}")
            print(f"  Model ID: {result.registers[0]}, Length: {result.registers[1]}")
            if result.registers[0] == 65535:
                print("  ✓ End marker found!")
            else:
                print("  ✗ End marker not found")
        else:
            print(f"  Error: {result}")
        
        # Check what's at the end of our register space
        print("\nEnd of Register Space (registers 9998-9999):")
        result = client.read_holding_registers(address=9998, count=2, slave=unit_id)
        if not result.isError():
            print(f"  Registers 9998-9999: {result.registers}")
        else:
            print(f"  Error: {result}")
        
        print("="*60)
        
    except Exception as e:
        print(f"Error: {e}")
    
    finally:
        if 'client' in locals():
            client.close()


if __name__ == "__main__":
    debug_registers()