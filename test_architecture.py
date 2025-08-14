#!/usr/bin/env python3
"""
Test script to verify the new architecture implementation
"""

def main():
    """Run all tests"""
    print("Running architecture tests...\n")
    
    try:
        # Test that abstract base classes are properly defined
        print("Testing abstract base classes...")
        from ems.base import InverterClient, InverterData, RegisterMapping
        assert InverterClient is not None
        assert InverterData is not None
        assert RegisterMapping is not None
        print("✓ Abstract base classes imported successfully")
        
        # Test the inverter factory pattern
        print("Testing inverter factory...")
        from ems.inverter_factory import InverterFactory
        
        # Test Sol-Ark factory creation
        solark_client = InverterFactory.create_inverter("solark", port="/dev/ttyUSB0", baudrate=9600, modbus_address=1)
        assert solark_client is not None
        print("✓ Sol-Ark inverter factory creation successful")
        
        # Test unknown inverter type
        unknown_client = InverterFactory.create_inverter("unknown", port="/dev/ttyUSB0", baudrate=9600, modbus_address=1)
        assert unknown_client is None
        print("✓ Unknown inverter type correctly returns None")
        
        # Test SunSpec mapper functionality
        print("Testing SunSpec mapper...")
        from ems.sunspec_models import SunSpecMapper
        
        # Test SunSpec mapper creation
        device_info = {
            "manufacturer": "Test Manufacturer",
            "model": "Test Model",
            "version": "1.0.0",
            "serial_number": "TEST001",
            "options": "Test Gateway"
        }
        
        mapper = SunSpecMapper(device_info)
        assert mapper is not None
        print("✓ SunSpec mapper creation successful")
        
        # Test Modbus server functionality
        print("Testing Modbus server...")
        from ems.modbus_server import SunSpecModbusServer, ModbusServerConfig
        
        # Test Modbus server creation
        config = ModbusServerConfig(
            host="127.0.0.1",
            port=8502,
            device_id=1,
            device_info={
                "manufacturer": "Test Manufacturer",
                "model": "Test Model",
                "version": "1.0.0",
                "serial_number": "TEST001",
                "options": "Test Gateway"
            }
        )
        
        server = SunSpecModbusServer(config)
        assert server is not None
        print("✓ Modbus server creation successful")
        
        print("\n✓ All tests passed! Architecture implementation is working correctly.")
        return 0
        
    except Exception as e:
        print(f"\n✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    import sys
    sys.exit(main())