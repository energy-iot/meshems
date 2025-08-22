#!/usr/bin/env python3
"""
Test script to verify SunSpec register mappings are correct according to model 701 specification.
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), 'ems'))

from ems.sunspec_models import SunSpecMapper, SunSpecRegisterMap
from ems.base import InverterData

def test_sunspec_register_mappings():
    """Test that SunSpec register mappings match the model 701 specification"""
    
    # Create test device info
    device_info = {
        "manufacturer": "Test Manufacturer",
        "model": "Test Model",
        "version": "1.0.0",
        "serial_number": "TEST001"
    }
    
    # Initialize mapper
    mapper = SunSpecMapper(device_info)
    
    # Create test inverter data with split-phase configuration
    test_data = InverterData()
    test_data.grid_type = 1  # Split-phase
    test_data.grid_power_l1 = 1000.0  # 1000W L1
    test_data.grid_power_l2 = 1500.0  # 1500W L2
    test_data.grid_current_l1 = 8.33   # 8.33A L1
    test_data.grid_current_l2 = 12.5   # 12.5A L2
    test_data.grid_voltage_l1n = 120.0 # 120V L1-N
    test_data.grid_voltage_l2n = 120.0 # 120V L2-N
    test_data.grid_voltage_l1l2 = 240.0 # 240V L1-L2
    
    # Update mapper with test data
    mapper.update_from_inverter_data(test_data)
    
    # Get registers
    registers = mapper.get_registers()
    
    # Test key register mappings according to SunSpec 701 specification
    grid_base = SunSpecRegisterMap.GRID_MODEL_BASE
    
    print("Testing SunSpec Model 701 Register Mappings:")
    print("=" * 50)
    
    # Test AL2 (Amps L2) - should be at offset 68
    al2_register = grid_base + 68
    al2_value = registers.get(al2_register, 0)
    expected_al2 = int(12.5 * 100)  # 12.5A with scale factor -2
    print(f"AL2 (Amps L2) at offset 68:")
    print(f"  Register {al2_register}: {al2_value} (expected: {expected_al2})")
    print(f"  ✓ PASS" if al2_value == expected_al2 else f"  ✗ FAIL")
    
    # Test AL1 (Amps L1) - should be at offset 45
    al1_register = grid_base + 45
    al1_value = registers.get(al1_register, 0)
    expected_al1 = int(8.33 * 100)  # 8.33A with scale factor -2
    print(f"\nAL1 (Amps L1) at offset 45:")
    print(f"  Register {al1_register}: {al1_value} (expected: {expected_al1})")
    print(f"  ✓ PASS" if al1_value == expected_al1 else f"  ✗ FAIL")
    
    # Test WL2 (Watts L2) - should be at offset 64
    wl2_register = grid_base + 64
    wl2_value = registers.get(wl2_register, 0)
    expected_wl2 = int(1500.0)  # 1500W with scale factor 0
    print(f"\nWL2 (Watts L2) at offset 64:")
    print(f"  Register {wl2_register}: {wl2_value} (expected: {expected_wl2})")
    print(f"  ✓ PASS" if wl2_value == expected_wl2 else f"  ✗ FAIL")
    
    # Test WL1 (Watts L1) - should be at offset 41
    wl1_register = grid_base + 41
    wl1_value = registers.get(wl1_register, 0)
    expected_wl1 = int(1000.0)  # 1000W with scale factor 0
    print(f"\nWL1 (Watts L1) at offset 41:")
    print(f"  Register {wl1_register}: {wl1_value} (expected: {expected_wl1})")
    print(f"  ✓ PASS" if wl1_value == expected_wl1 else f"  ✗ FAIL")
    
    # Test VL2 (Phase Voltage L2-N) - should be at offset 70
    vl2_register = grid_base + 70
    vl2_value = registers.get(vl2_register, 0)
    expected_vl2 = int(120.0 * 10)  # 120V with scale factor -1
    print(f"\nVL2 (Phase Voltage L2-N) at offset 70:")
    print(f"  Register {vl2_register}: {vl2_value} (expected: {expected_vl2})")
    print(f"  ✓ PASS" if vl2_value == expected_vl2 else f"  ✗ FAIL")
    
    # Test VL1 (Phase Voltage L1-N) - should be at offset 47
    vl1_register = grid_base + 47
    vl1_value = registers.get(vl1_register, 0)
    expected_vl1 = int(120.0 * 10)  # 120V with scale factor -1
    print(f"\nVL1 (Phase Voltage L1-N) at offset 47:")
    print(f"  Register {vl1_register}: {vl1_value} (expected: {expected_vl1})")
    print(f"  ✓ PASS" if vl1_value == expected_vl1 else f"  ✗ FAIL")
    
    # Test VL1L2 (Phase Voltage L1-L2) - should be at offset 46
    vl1l2_register = grid_base + 46
    vl1l2_value = registers.get(vl1l2_register, 0)
    expected_vl1l2 = int(240.0 * 10)  # 240V with scale factor -1
    print(f"\nVL1L2 (Phase Voltage L1-L2) at offset 46:")
    print(f"  Register {vl1l2_register}: {vl1l2_value} (expected: {expected_vl1l2})")
    print(f"  ✓ PASS" if vl1l2_value == expected_vl1l2 else f"  ✗ FAIL")
    
    print("\n" + "=" * 50)
    print("Register mapping test completed!")
    
    # Verify that old incorrect registers are not being set
    print("\nVerifying old incorrect registers are not set:")
    old_al2_register = grid_base + 26  # Old incorrect offset
    old_al2_value = registers.get(old_al2_register, 0xFFFF)
    print(f"Old AL2 register at offset 26: {old_al2_value} (should be 0xFFFF or 0x8000)")
    print(f"  ✓ PASS" if old_al2_value in [0xFFFF, 0x8000] else f"  ✗ FAIL - still using old offset!")

if __name__ == "__main__":
    test_sunspec_register_mappings()