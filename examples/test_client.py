#!/usr/bin/env python3
"""
Test client for EMS-Dev Python Gateway

This script tests the Sol-Ark Modbus RTU client functionality.
"""

import sys
import time
import logging
from pathlib import Path

# Add parent directory to path to import ems module
sys.path.insert(0, str(Path(__file__).parent.parent))

from ems.solark_client import SolArkModbusClient


def test_solark_client(port: str = "/dev/ttyUSB0", address: int = 1):
    """Test Sol-Ark Modbus client"""
    
    # Setup logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    logger = logging.getLogger(__name__)
    
    try:
        # Create client
        logger.info(f"Creating Sol-Ark client on {port}, address {address}")
        client = SolArkModbusClient(port=port, modbus_address=address)
        
        # Connect
        if not client.connect():
            logger.error("Failed to connect to Sol-Ark")
            return False
        
        logger.info("Connected to Sol-Ark successfully")
        
        # Poll data
        logger.info("Polling Sol-Ark data...")
        if client.poll():
            logger.info("Poll successful!")
            
            # Display data
            print("\n" + "="*60)
            print("SOL-ARK INVERTER DATA")
            print("="*60)
            
            # Battery status
            print(f"\nBATTERY STATUS:")
            print(f"  Power:       {client.data.battery_power:.1f} W")
            print(f"  Current:     {client.data.battery_current:.2f} A")
            print(f"  Voltage:     {client.data.battery_voltage:.2f} V")
            print(f"  SOC:         {client.data.battery_soc:.0f}%")
            print(f"  Temperature: {client.data.battery_temperature:.1f}°C ({client.get_battery_temperature_f():.1f}°F)")
            print(f"  Capacity:    {client.data.battery_capacity:.1f} Ah")
            
            # Status
            print(f"  Status:      ", end="")
            if client.is_battery_charging():
                print("CHARGING")
            elif client.is_battery_discharging():
                print("DISCHARGING")
            else:
                print("IDLE")
            
            # Grid status
            print(f"\nGRID STATUS:")
            print(f"  Power:       {client.data.grid_power:.1f} W")
            print(f"  Voltage:     {client.data.grid_voltage:.1f} V")
            print(f"  Current L1:  {client.data.grid_current_l1:.2f} A")
            print(f"  Current L2:  {client.data.grid_current_l2:.2f} A")
            print(f"  Frequency:   {client.data.grid_frequency:.2f} Hz")
            
            print(f"  Connection:  ", end="")
            if client.is_grid_connected():
                print("CONNECTED")
                if client.is_selling_to_grid():
                    print("  Flow:        SELLING TO GRID")
                elif client.is_buying_from_grid():
                    print("  Flow:        BUYING FROM GRID")
                else:
                    print("  Flow:        NO POWER FLOW")
            else:
                print("DISCONNECTED")
            
            # Solar PV status
            print(f"\nSOLAR PV STATUS:")
            print(f"  PV1 Power:   {client.data.pv1_power:.1f} W")
            print(f"  PV2 Power:   {client.data.pv2_power:.1f} W")
            print(f"  Total Power: {client.data.pv1_power + client.data.pv2_power:.1f} W")
            print(f"  Total Power: {client.data.pv_power_total:.3f} kW")
            
            # Load status
            print(f"\nLOAD STATUS:")
            print(f"  Load L1:     {client.data.load_power_l1:.1f} W")
            print(f"  Load L2:     {client.data.load_power_l2:.1f} W")
            print(f"  Total Load:  {client.data.load_power_total:.1f} W")
            print(f"  Smart Load:  {client.data.smart_load_power:.1f} W")
            print(f"  Frequency:   {client.data.load_frequency:.2f} Hz")
            
            # Energy meters
            print(f"\nENERGY METERS (kWh):")
            print(f"  Battery Charge:    {client.data.battery_charge_energy:.1f} kWh")
            print(f"  Battery Discharge: {client.data.battery_discharge_energy:.1f} kWh")
            print(f"  Grid Buy:          {client.data.grid_buy_energy:.1f} kWh")
            print(f"  Grid Sell:         {client.data.grid_sell_energy:.1f} kWh")
            print(f"  Load:              {client.data.load_energy:.1f} kWh")
            print(f"  PV Generation:     {client.data.pv_energy:.1f} kWh")
            
            # Inverter details
            print(f"\nINVERTER DETAILS:")
            print(f"  Comm Version: {client.data.comm_version}")
            print(f"  Serial No:    {client.get_serial_number()}")
            print(f"  Grid Type:    {client.data.grid_type} (0:Single, 1:Split, 2:Three-Phase)")
            print(f"  Inv Status:   {client.data.inverter_status} (1:Self-test, 2:Normal, 3:Alarm, 4:Fault)")
            print(f"  DCDC Temp:    {client.data.dcdc_xfrmr_temp:.1f}°C")
            print(f"  IGBT Temp:    {client.data.igbt_temp:.1f}°C")
            
            # BMS data
            if client.data.bms_real_time_soc > 0:
                print(f"\nBMS DATA:")
                print(f"  BMS SOC:      {client.data.bms_real_time_soc:.1f}%")
                print(f"  BMS Voltage:  {client.data.bms_real_time_voltage:.2f} V")
                print(f"  BMS Current:  {client.data.bms_real_time_current:.2f} A")
                print(f"  BMS Temp:     {client.data.bms_real_time_temp:.1f}°C")
                print(f"  BMS Warning:  0x{client.data.bms_warning:04X}")
                print(f"  BMS Fault:    0x{client.data.bms_fault:04X}")
            
            print("\n" + "="*60)
            
            return True
        else:
            logger.error("Failed to poll Sol-Ark data")
            return False
    
    except Exception as e:
        logger.error(f"Error testing Sol-Ark client: {e}")
        return False
    
    finally:
        # Disconnect
        client.disconnect()


def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Test Sol-Ark Modbus client")
    parser.add_argument("--port", "-p", default="/dev/ttyUSB0", help="Serial port")
    parser.add_argument("--address", "-a", type=int, default=1, help="Modbus address")
    parser.add_argument("--continuous", "-c", action="store_true", help="Continuous polling")
    parser.add_argument("--interval", "-i", type=float, default=5.0, help="Poll interval (seconds)")
    
    args = parser.parse_args()
    
    if args.continuous:
        print(f"Starting continuous polling every {args.interval} seconds...")
        print("Press Ctrl+C to stop")
        
        try:
            while True:
                success = test_solark_client(args.port, args.address)
                if not success:
                    print("Poll failed, retrying...")
                
                time.sleep(args.interval)
        
        except KeyboardInterrupt:
            print("\nStopped by user")
    
    else:
        # Single test
        success = test_solark_client(args.port, args.address)
        sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()