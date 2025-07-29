#!/usr/bin/env python3
"""
Test SunSpec server for EMS-Dev Python Gateway

This script tests the SunSpec Modbus TCP server functionality.
"""

import sys
import time
import logging
from pathlib import Path

# Add parent directory to path to import ems module
sys.path.insert(0, str(Path(__file__).parent.parent))

from ems.modbus_server import SunSpecModbusServer, ModbusServerConfig
from ems.solark_client import SolArkData


def create_test_data() -> SolArkData:
    """Create test Sol-Ark data"""
    data = SolArkData()
    
    # Battery data
    data.battery_power = -1500.0  # Charging
    data.battery_current = -25.0
    data.battery_voltage = 52.4
    data.battery_soc = 85.0
    data.battery_temperature = 25.5
    data.battery_capacity = 200.0
    
    # Grid data
    data.grid_power = 2500.0  # Buying from grid
    data.grid_voltage = 240.0
    data.grid_current_l1 = 10.5
    data.grid_current_l2 = 10.0
    data.grid_frequency = 60.02
    data.grid_relay_status = 1  # Connected
    
    # PV data
    data.pv1_power = 1800.0
    data.pv2_power = 1200.0
    data.pv_power_total = 3.0  # kW
    data.pv_energy = 25.5  # kWh today
    
    # Load data
    data.load_power_l1 = 800.0
    data.load_power_l2 = 700.0
    data.load_power_total = 1500.0
    data.load_frequency = 60.01
    
    # Inverter data
    data.inverter_voltage = 240.0
    data.inverter_current_l1 = 3.3
    data.inverter_current_l2 = 2.9
    data.inverter_frequency = 60.01
    data.inverter_status = 2  # Normal
    data.inverter_output_power = 1500.0
    
    # Energy counters
    data.battery_charge_energy = 15.2
    data.battery_discharge_energy = 8.7
    data.grid_buy_energy = 45.3
    data.grid_sell_energy = 12.1
    data.load_energy = 38.9
    
    # Temperatures
    data.dcdc_xfrmr_temp = 45.2
    data.igbt_temp = 42.8
    
    # Device info
    data.comm_version = 130
    data.serial_number_parts = [0x5341, 0x524B, 0x3031, 0x3233, 0x3435]  # "SARK01235"
    data.grid_type = 1  # Split phase
    
    # BMS data
    data.bms_real_time_soc = 85.2
    data.bms_real_time_voltage = 52.3
    data.bms_real_time_current = -24.8
    data.bms_real_time_temp = 26.1
    data.bms_warning = 0x0000
    data.bms_fault = 0x0000
    
    data.last_update = time.time()
    
    return data


def test_sunspec_server(host: str = "localhost", port: int = 8502):
    """Test SunSpec Modbus server"""
    
    # Setup logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    logger = logging.getLogger(__name__)
    
    try:
        # Create server configuration
        device_info = {
            "manufacturer": "Energy IoT Open Source",
            "model": "EMS-Dev Python Test",
            "version": "1.0.0",
            "serial_number": "TEST-001",
            "options": "Sol-Ark Gateway Test"
        }
        
        config = ModbusServerConfig(
            host=host,
            port=port,
            unit_id=1,
            device_info=device_info
        )
        
        # Create server
        logger.info(f"Creating SunSpec server on {host}:{port}")
        server = SunSpecModbusServer(config)
        
        # Start server
        logger.info("Starting SunSpec server...")
        server.start()
        
        # Wait for server to start
        time.sleep(2)
        
        if not server.is_running():
            logger.error("Server failed to start")
            return False
        
        logger.info("Server started successfully")
        
        # Create test data
        test_data = create_test_data()
        
        # Update server with test data
        logger.info("Updating server with test data...")
        server.update_from_solark(test_data)
        
        # Display server statistics
        stats = server.get_statistics()
        print("\n" + "="*60)
        print("SUNSPEC MODBUS SERVER STATUS")
        print("="*60)
        print(f"Running:        {stats['running']}")
        print(f"Host:           {stats['host']}")
        print(f"Port:           {stats['port']}")
        print(f"Unit ID:        {stats['unit_id']}")
        print(f"Register Count: {stats['register_count']}")
        
        # Display some register values
        print(f"\nSAMPLE REGISTER VALUES:")
        
        # SunSpec ID
        sunspec_id_high = server.get_register_value(40000)
        sunspec_id_low = server.get_register_value(40001)
        if sunspec_id_high is not None and sunspec_id_low is not None:
            sunspec_id = (sunspec_id_high << 16) | sunspec_id_low
            print(f"SunSpec ID:     0x{sunspec_id:08X} ({'SunS' if sunspec_id == 0x53756E53 else 'Invalid'})")
        
        # Common model
        model_id = server.get_register_value(40002)
        model_length = server.get_register_value(40003)
        print(f"Common Model:   ID={model_id}, Length={model_length}")
        
        # Inverter model
        inv_model_id = server.get_register_value(40100)
        inv_model_length = server.get_register_value(40101)
        print(f"Inverter Model: ID={inv_model_id}, Length={inv_model_length}")
        
        # Battery model
        bat_model_id = server.get_register_value(40200)
        bat_model_length = server.get_register_value(40201)
        print(f"Battery Model:  ID={bat_model_id}, Length={bat_model_length}")
        
        # Some data values
        ac_power = server.get_register_value(40106)
        dc_voltage = server.get_register_value(40111)
        battery_soc = server.get_register_value(40205)
        
        print(f"\nDATA VALUES:")
        print(f"AC Power:       {ac_power} W")
        print(f"DC Voltage:     {dc_voltage/10:.1f} V (scaled)")
        print(f"Battery SOC:    {battery_soc}%")
        
        print(f"\nSERVER READY FOR CONNECTIONS")
        print(f"Connect SunSpec clients to {host}:{port}")
        print(f"Example: python examples/sunspec_client_example.py {host} {port}")
        print(f"\nPress Ctrl+C to stop server...")
        
        # Keep server running
        try:
            while True:
                time.sleep(1)
                
                # Update with new test data periodically
                if int(time.time()) % 10 == 0:  # Every 10 seconds
                    # Simulate changing data
                    test_data.battery_soc = 85.0 + (time.time() % 30) / 30 * 10  # 85-95%
                    test_data.pv1_power = 1800.0 + (time.time() % 60) / 60 * 400  # 1800-2200W
                    test_data.last_update = time.time()
                    
                    server.update_from_solark(test_data)
                    logger.debug("Updated server with new test data")
        
        except KeyboardInterrupt:
            logger.info("Stopping server...")
        
        return True
    
    except Exception as e:
        logger.error(f"Error testing SunSpec server: {e}")
        return False
    
    finally:
        # Stop server
        if 'server' in locals():
            server.stop()
            logger.info("Server stopped")


def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Test SunSpec Modbus server")
    parser.add_argument("--host", "-H", default="localhost", help="Server host")
    parser.add_argument("--port", "-p", type=int, default=8502, help="Server port")
    
    args = parser.parse_args()
    
    success = test_sunspec_server(args.host, args.port)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()