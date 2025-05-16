#!/usr/bin/env python3
"""
SunSpec Client Example for Sol-Ark Inverter

This script demonstrates how to use the pysunspec2 library to read data from
the SunSpec-compliant Modbus TCP server implemented in the EMS-Dev platform
for the Sol-Ark inverter.

Requirements:
- Python 3.6+
- pysunspec2 (pip install pysunspec2)

Usage:
python sunspec_client_example.py [ip_address] [port]

Example:
python sunspec_client_example.py 192.168.1.100 8502
"""

import sys
import time
import sunspec2.modbus.client as client

def main():
    # Parse command line arguments
    if len(sys.argv) < 2:
        print("Usage: python sunspec_client_example.py [ip_address] [port]")
        print("Example: python sunspec_client_example.py 192.168.1.100 8502")
        sys.exit(1)
    
    ip_address = sys.argv[1]
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 8502
    
    try:
        # Create a Modbus TCP client
        print(f"Connecting to SunSpec device at {ip_address}:{port}...")
        modbus_device = client.SunSpecModbusClientDeviceTCP(
            ipaddr=ip_address,
            ipport=port,
            timeout=5
        )
        
        # Scan for SunSpec models
        print("Scanning for SunSpec models...")
        modbus_device.scan()
        
        # Check if SunSpec device was found
        if not modbus_device.models:
            print("No SunSpec models found. Check connection and IP address.")
            sys.exit(1)
        
        # Print device information from Common Model (1)
        if 1 in modbus_device.models:
            common = modbus_device.models[1]
            print("\nDevice Information (Common Model):")
            print(f"Manufacturer: {common.Mn.value}")
            print(f"Model: {common.Md.value}")
            print(f"Options: {common.Opt.value}")
            print(f"Version: {common.Vr.value}")
            print(f"Serial Number: {common.SN.value}")
        
        # Print inverter data from Inverter Model (701)
        if 701 in modbus_device.models:
            inverter = modbus_device.models[701]
            
            # Read current values
            inverter.read()
            
            print("\nInverter Data (Model 701):")
            
            # AC measurements
            print("\nAC Measurements:")
            print(f"AC Current: {inverter.A.value} A")
            print(f"AC Current Phase A: {inverter.AphA.value} A")
            print(f"AC Current Phase B: {inverter.AphB.value} A")
            print(f"AC Voltage (L-L): {inverter.PPVphAB.value} V")
            print(f"AC Power: {inverter.W.value} W")
            print(f"AC Frequency: {inverter.Hz.value} Hz")
            print(f"AC Energy: {inverter.WH.value} Wh")
            
            # DC measurements
            print("\nDC Measurements:")
            print(f"DC Current: {inverter.DCA.value} A")
            print(f"DC Voltage: {inverter.DCV.value} V")
            print(f"DC Power: {inverter.DCW.value} W")
            
            # Temperature
            print("\nTemperature:")
            print(f"Cabinet Temperature: {inverter.TmpCab.value} °C")
            
            # Status
            print("\nStatus:")
            status = inverter.St.value
            status_text = "Unknown"
            
            if status & 0x0001:
                status_text = "Off"
            elif status & 0x0002:
                status_text = "Sleeping"
            elif status & 0x0004:
                status_text = "Starting"
            elif status & 0x0008:
                status_text = "MPPT"
            elif status & 0x0010:
                status_text = "Throttled"
            elif status & 0x0020:
                status_text = "Shutting Down"
            elif status & 0x0040:
                status_text = "Fault"
            elif status & 0x0080:
                status_text = "Standby"
            
            print(f"Inverter Status: {status_text} (0x{status:04X})")
            
            # Vendor-specific status
            vendor_status = inverter.StVnd.value
            print("\nVendor Status:")
            print(f"Grid Connected: {'Yes' if vendor_status & 0x0001 else 'No'}")
            print(f"Generator Connected: {'Yes' if vendor_status & 0x0002 else 'No'}")
            print(f"Battery Charging: {'Yes' if vendor_status & 0x0004 else 'No'}")
            print(f"Battery Discharging: {'Yes' if vendor_status & 0x0008 else 'No'}")
            print(f"Selling to Grid: {'Yes' if vendor_status & 0x0010 else 'No'}")
            print(f"Buying from Grid: {'Yes' if vendor_status & 0x0020 else 'No'}")
        else:
            print("Inverter Model (701) not found.")
        
        # Continuous monitoring loop
        print("\nStarting continuous monitoring (press Ctrl+C to exit)...")
        try:
            while True:
                if 701 in modbus_device.models:
                    inverter = modbus_device.models[701]
                    inverter.read()
                    
                    print("\n--- Inverter Data Update ---")
                    print(f"AC Power: {inverter.W.value} W")
                    print(f"DC Power: {inverter.DCW.value} W")
                    print(f"Battery Voltage: {inverter.DCV.value} V")
                    print(f"Battery Current: {inverter.DCA.value} A")
                    
                    # Vendor status interpretation
                    vendor_status = inverter.StVnd.value
                    if vendor_status & 0x0004:
                        print("Battery Status: CHARGING")
                    elif vendor_status & 0x0008:
                        print("Battery Status: DISCHARGING")
                    else:
                        print("Battery Status: IDLE")
                    
                    if vendor_status & 0x0010:
                        print("Grid Status: SELLING")
                    elif vendor_status & 0x0020:
                        print("Grid Status: BUYING")
                    else:
                        print("Grid Status: BALANCED")
                
                time.sleep(5)  # Update every 5 seconds
        
        except KeyboardInterrupt:
            print("\nMonitoring stopped by user.")
    
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
    
    finally:
        # Close the connection
        if 'modbus_device' in locals():
            modbus_device.close()
            print("Connection closed.")

if __name__ == "__main__":
    main()
