#!/usr/bin/env python3
"""
Test script for SunSpec 714 model implementation
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), 'ems'))

from ems.sunspec_models import SunSpecMapper, SunSpec714Model, SunSpecRegisterMap
from ems.solark_client import SolArkData

def test_sunspec_714_model():
    """Test the SunSpec 714 model implementation"""
    print("Testing SunSpec 714 Model Implementation")
    print("=" * 50)
    
    # Create device info
    device_info = {
        "manufacturer": "Energy IoT Open Source",
        "model": "EMS-Dev Python",
        "version": "1.0.0",
        "serial_number": "EMS-PY-714-TEST"
    }
    
    # Initialize SunSpec mapper
    mapper = SunSpecMapper(device_info)
    
    # Test 1: Verify DC model initialization
    print("Test 1: DC Model Initialization")
    print(f"  Model ID: {mapper.dc_model.model_id}")
    print(f"  Model Length: {mapper.dc_model.model_length}")
    print(f"  Number of Ports: {mapper.dc_model.num_ports}")
    print(f"  Port Types: {[port.port_type for port in mapper.dc_model.ports]}")
    print(f"  Port IDs: {[port.port_id_string for port in mapper.dc_model.ports]}")
    
    # Test 2: Verify register mapping
    print("\nTest 2: Register Mapping")
    print(f"  DC Model Base Address: {SunSpecRegisterMap.DC_MODEL_BASE}")
    print(f"  DC Model ID Register: {SunSpecRegisterMap.DC_MODEL_ID}")
    print(f"  DC Model Length Register: {SunSpecRegisterMap.DC_MODEL_LENGTH}")
    print(f"  Port 1 Base: {SunSpecRegisterMap.DC_PORT1_BASE}")
    print(f"  Port 4 Base (uninitialized): {SunSpecRegisterMap.DC_PORT4_BASE}")
    print(f"  Port 5 Base (ESS): {SunSpecRegisterMap.DC_PORT5_BASE}")
    
    # Test 3: Verify register values
    print("\nTest 3: Register Values")
    model_id_reg = mapper.get_register_value(SunSpecRegisterMap.DC_MODEL_ID)
    model_length_reg = mapper.get_register_value(SunSpecRegisterMap.DC_MODEL_LENGTH)
    num_ports_reg = mapper.get_register_value(SunSpecRegisterMap.DC_NUM_PORTS)
    
    print(f"  Model ID Register Value: {model_id_reg}")
    print(f"  Model Length Register Value: {model_length_reg}")
    print(f"  Number of Ports Register Value: {num_ports_reg}")
    
    # Test 4: Verify Port 4 is uninitialized
    print("\nTest 4: Port 4 Uninitialized Values")
    port4_base = SunSpecRegisterMap.DC_PORT4_BASE
    port4_current = mapper.get_register_value(port4_base + 10)  # DCA
    port4_voltage = mapper.get_register_value(port4_base + 11)  # DCV
    port4_power = mapper.get_register_value(port4_base + 12)    # DCW
    port4_temp = mapper.get_register_value(port4_base + 21)     # Tmp
    port4_status = mapper.get_register_value(port4_base + 22)   # DCSta
    
    print(f"  Port 4 Current (should be 0xFFFF): 0x{port4_current:04X}")
    print(f"  Port 4 Voltage (should be 0xFFFF): 0x{port4_voltage:04X}")
    print(f"  Port 4 Power (should be 0xFFFF): 0x{port4_power:04X}")
    print(f"  Port 4 Temperature (should be 0x8000): 0x{port4_temp:04X}")
    print(f"  Port 4 Status (should be 0xFFFF): 0x{port4_status:04X}")
    
    # Test 5: Test with mock Sol-Ark data
    print("\nTest 5: Mock Sol-Ark Data Update")
    
    # Create mock Sol-Ark data
    class MockSolArkData:
        def __init__(self):
            # Grid data
            self.grid_current_l1 = 5.2
            self.grid_current_l2 = 4.8
            self.grid_voltage = 240.5
            self.grid_power = 2400
            self.grid_frequency = 60.02
            self.grid_sell_energy = 12.5
            self.grid_buy_energy = 8.3
            self.grid_relay_status = 2
            self.grid_type = 0x01  # Split-phase
            
            # Load data
            self.load_current_l1 = 8.1
            self.load_current_l2 = 7.9
            self.load_power_total = 3800
            self.load_power_l1 = 1900
            self.load_power_l2 = 1900
            self.load_frequency = 60.01
            self.load_energy = 45.2
            
            # Inverter data
            self.inverter_voltage = 240.2
            self.inverter_voltage_ln = 120.1
            self.inverter_voltage_l2n = 120.1
            self.inverter_current_l1 = 8.0
            self.inverter_current_l2 = 7.8
            self.inverter_power_l1 = 960
            self.inverter_power_l2 = 936
            self.inverter_output_power = 1896
            self.inverter_frequency = 60.00
            self.inverter_status = 2  # Normal
            
            # Battery data
            self.battery_voltage = 52.4
            self.battery_current = -15.2  # Charging
            self.battery_power = -796.5   # Charging
            self.battery_soc = 85.5
            self.battery_temperature = 23.8
            self.battery_capacity = 200.0
            
            # Temperature data
            self.igbt_temp = 45.2
            self.dcdc_xfrmr_temp = 38.7
            
            # BMS data
            self.bms_fault = 0
            self.bms_warning = 0
            
            # Generator data
            self.generator_relay_status = 0
            
            # PV data (mock - these might not exist in real SolArkData)
            self.pv1_voltage = 385.2
            self.pv1_current = 8.5
            self.pv1_power = 3274
            
            self.pv2_voltage = 392.1
            self.pv2_current = 7.8
            self.pv2_power = 3058
            
            self.pv3_voltage = 0.0  # Not connected
            self.pv3_current = 0.0
            self.pv3_power = 0.0
    
    mock_data = MockSolArkData()
    
    # Update mapper with mock data
    mapper.update_from_solark(mock_data)
    
    # Test 6: Verify updated values
    print("\nTest 6: Updated DC Model Values")
    print(f"  Total DC Current: {mapper.dc_model.total_dc_current:.2f} A")
    print(f"  Total DC Power: {mapper.dc_model.total_dc_power:.2f} W")
    
    print("\n  Port Values:")
    for i, port in enumerate(mapper.dc_model.ports):
        port_type = "PV" if port.port_type == 0 else "ESS"
        if i == 3:  # Port 4 - uninitialized
            print(f"    Port {i+1} ({port_type}): UNINITIALIZED")
        else:
            print(f"    Port {i+1} ({port_type}): {port.dc_voltage:.1f}V, {port.dc_current:.1f}A, {port.dc_power:.0f}W, Status: {port.dc_status}")
    
    # Test 7: Verify register updates
    print("\nTest 7: Updated Register Values")
    total_current_reg = mapper.get_register_value(SunSpecRegisterMap.DC_TOTAL_CURRENT)
    total_power_reg = mapper.get_register_value(SunSpecRegisterMap.DC_TOTAL_POWER)
    
    print(f"  Total Current Register: {total_current_reg} (scaled)")
    print(f"  Total Power Register: {total_power_reg}")
    
    # Test Port 1 registers
    port1_base = SunSpecRegisterMap.DC_PORT1_BASE
    port1_voltage_reg = mapper.get_register_value(port1_base + 11)
    port1_current_reg = mapper.get_register_value(port1_base + 10)
    port1_power_reg = mapper.get_register_value(port1_base + 12)
    
    print(f"  Port 1 Voltage Register: {port1_voltage_reg} (scaled)")
    print(f"  Port 1 Current Register: {port1_current_reg} (scaled)")
    print(f"  Port 1 Power Register: {port1_power_reg}")
    
    print("\n" + "=" * 50)
    print("SunSpec 714 Model Test Completed Successfully!")
    
    return True

if __name__ == "__main__":
    test_sunspec_714_model()