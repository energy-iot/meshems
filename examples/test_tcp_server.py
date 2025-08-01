#!/usr/bin/env python3
"""
Test Modbus TCP Server Client

This script tests the SunSpec Modbus TCP server by connecting and reading registers.
Compatible with pymodbus 3.5.0+
"""

import sys
import time
import logging
from pathlib import Path

# Add parent directory to path to import ems module
sys.path.insert(0, str(Path(__file__).parent.parent))

from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException


def test_modbus_tcp_server(host: str = "localhost", port: int = 8502, unit_id: int = 1):
    """Test Modbus TCP server connection and register reading"""
    
    # Setup logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    logger = logging.getLogger(__name__)
    
    try:
        # Create Modbus TCP client
        logger.info(f"Connecting to Modbus TCP server at {host}:{port}")
        client = ModbusTcpClient(host=host, port=port, timeout=5)
        
        # Connect to server
        if not client.connect():
            logger.error("Failed to connect to Modbus TCP server")
            return False
        
        logger.info("Connected to Modbus TCP server successfully")
        
        # Test reading SunSpec identification registers
        logger.info("Reading SunSpec identification...")
        
        # Read SunSpec header (registers 40000-40001)
        result = client.read_holding_registers(address=0, count=2, slave=unit_id)
        if not result.isError():
            sunspec_id = (result.registers[0] << 16) | result.registers[1]
            if sunspec_id == 0x53756E53:  # "SunS" in ASCII
                logger.info("✓ SunSpec header found: 'SunS'")
            else:
                logger.warning(f"Unexpected SunSpec header: 0x{sunspec_id:08X}")
        else:
            logger.error(f"Failed to read SunSpec header: {result}")
        
        # Read Common Model (Model 1) - typically starts at register 40002
        logger.info("Reading Common Model (Model 1)...")
        result = client.read_holding_registers(address=2, count=65, slave=unit_id)
        if not result.isError():
            registers = result.registers
            
            # Parse Common Model
            model_id = registers[0]
            model_length = registers[1]
            
            logger.info(f"✓ Model ID: {model_id}, Length: {model_length}")
            
            if model_id == 1:  # Common Model
                # Extract manufacturer (registers 2-17, 16 chars)
                manufacturer = ""
                for i in range(2, 18):
                    if i < len(registers):
                        char1 = (registers[i] >> 8) & 0xFF
                        char2 = registers[i] & 0xFF
                        if char1 != 0:
                            manufacturer += chr(char1)
                        if char2 != 0:
                            manufacturer += chr(char2)
                
                # Extract model (registers 18-33, 16 chars)
                model = ""
                for i in range(18, 34):
                    if i < len(registers):
                        char1 = (registers[i] >> 8) & 0xFF
                        char2 = registers[i] & 0xFF
                        if char1 != 0:
                            model += chr(char1)
                        if char2 != 0:
                            model += chr(char2)
                
                # Extract version (registers 42-49, 8 chars)
                version = ""
                for i in range(42, 50):
                    if i < len(registers):
                        char1 = (registers[i] >> 8) & 0xFF
                        char2 = registers[i] & 0xFF
                        if char1 != 0:
                            version += chr(char1)
                        if char2 != 0:
                            version += chr(char2)
                
                print("\n" + "="*60)
                print("SUNSPEC DEVICE INFORMATION")
                print("="*60)
                print(f"Manufacturer: {manufacturer.strip()}")
                print(f"Model:        {model.strip()}")
                print(f"Version:      {version.strip()}")
                print("="*60)
            
        else:
            logger.error(f"Failed to read Common Model: {result}")
        
        # Test reading some sample holding registers
        logger.info("Testing register reads...")
        
        # Read a block of registers
        test_addresses = [0, 10, 50, 100, 200]
        for addr in test_addresses:
            result = client.read_holding_registers(address=addr, count=10, slave=unit_id)
            if not result.isError():
                logger.info(f"✓ Registers {addr}-{addr+9}: {result.registers}")
            else:
                logger.warning(f"✗ Failed to read registers {addr}-{addr+9}: {result}")
        
        # Test reading input registers
        logger.info("Testing input register reads...")
        result = client.read_input_registers(address=0, count=10, slave=unit_id)
        if not result.isError():
            logger.info(f"✓ Input registers 0-9: {result.registers}")
        else:
            logger.warning(f"✗ Failed to read input registers: {result}")
        
        # Test reading coils
        logger.info("Testing coil reads...")
        result = client.read_coils(address=0, count=10, slave=unit_id)
        if not result.isError():
            logger.info(f"✓ Coils 0-9: {result.bits}")
        else:
            logger.warning(f"✗ Failed to read coils: {result}")
        
        # Test reading discrete inputs
        logger.info("Testing discrete input reads...")
        result = client.read_discrete_inputs(address=0, count=10, slave=unit_id)
        if not result.isError():
            logger.info(f"✓ Discrete inputs 0-9: {result.bits}")
        else:
            logger.warning(f"✗ Failed to read discrete inputs: {result}")
        
        logger.info("✓ All tests completed successfully")
        return True
        
    except ModbusException as e:
        logger.error(f"Modbus exception: {e}")
        return False
    except Exception as e:
        logger.error(f"Error testing Modbus TCP server: {e}")
        return False
    
    finally:
        # Close connection
        if 'client' in locals():
            client.close()
            logger.info("Connection closed")


def continuous_poll(host: str, port: int, unit_id: int, interval: float):
    """Continuously poll the server"""
    logger = logging.getLogger(__name__)
    
    try:
        client = ModbusTcpClient(host=host, port=port, timeout=5)
        
        if not client.connect():
            logger.error("Failed to connect for continuous polling")
            return
        
        logger.info(f"Starting continuous polling every {interval} seconds...")
        logger.info("Press Ctrl+C to stop")
        
        while True:
            try:
                # Read some key registers
                result = client.read_holding_registers(address=0, count=20, slave=unit_id)
                if not result.isError():
                    timestamp = time.strftime("%H:%M:%S")
                    print(f"[{timestamp}] Registers 0-19: {result.registers[:10]}...")
                else:
                    print(f"[{timestamp}] Read failed: {result}")
                
                time.sleep(interval)
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                logger.error(f"Error during continuous polling: {e}")
                time.sleep(interval)
    
    except KeyboardInterrupt:
        pass
    finally:
        if 'client' in locals():
            client.close()
        print("\nContinuous polling stopped")


def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Test Modbus TCP server")
    parser.add_argument("--host", "-H", default="localhost", help="Server host (default: localhost)")
    parser.add_argument("--port", "-p", type=int, default=8502, help="Server port (default: 8502)")
    parser.add_argument("--unit", "-u", type=int, default=1, help="Unit ID (default: 1)")
    parser.add_argument("--continuous", "-c", action="store_true", help="Continuous polling")
    parser.add_argument("--interval", "-i", type=float, default=5.0, help="Poll interval (seconds)")
    
    args = parser.parse_args()
    
    if args.continuous:
        continuous_poll(args.host, args.port, args.unit, args.interval)
    else:
        # Single test
        success = test_modbus_tcp_server(args.host, args.port, args.unit)
        sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()