# EMS-Dev Python Gateway Architecture

## Overview

This document describes the architecture for expanding the EMS-Dev Python Gateway to support multiple inverter types with different Modbus register mappings and both split phase and 3-phase inverters. The architecture maintains backward compatibility with existing Sol-Ark installations while providing flexibility for other inverter types.

## Current Architecture Analysis

The current system is specifically designed for Sol-Ark inverters with the following components:

1. `SolArkModbusClient` - Handles Modbus RTU communication
2. `SolArkData` - Data structure for inverter data
3. `SolArkRegisterMap` - Register mappings and scaling factors
4. `SunSpecMapper` - Maps inverter data to SunSpec models

## Proposed Architecture

### Abstract Interface Design

The architecture introduces abstract base classes to support multiple inverter types:

```python
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Dict, List, Any
import time

@dataclass
class InverterData(ABC):
    """Abstract base class for inverter data"""
    timestamp: float = time.time()
    grid_type: int = 0  # 0=Single, 1=Split, 2=Three-phase
    
    # Grid measurements
    grid_power: float = 0.0
    grid_voltage_l1l2: float = 0.0
    grid_voltage_l1n: float = 0.0
    grid_voltage_l2n: float = 0.0
    grid_current_l1: float = 0.0
    grid_current_l2: float = 0.0
    grid_frequency: float = 0.0
    
    # Battery measurements
    battery_power: float = 0.0
    battery_voltage: float = 0.0
    battery_current: float = 0.0
    battery_soc: float = 0.0
    
    # PV measurements
    pv_power_total: float = 0.0
    pv1_power: float = 0.0
    pv2_power: float = 0.0

class InverterClient(ABC):
    """Abstract base class for inverter communication"""
    
    def __init__(self, port: str, baudrate: int, modbus_address: int):
        self.port = port
        self.baudrate = baudrate
        self.modbus_address = modbus_address
        self.data = InverterData()
    
    @abstractmethod
    def connect(self) -> bool:
        """Connect to the inverter"""
        pass
    
    @abstractmethod
    def disconnect(self) -> None:
        """Disconnect from the inverter"""
        pass
    
    @abstractmethod
    def poll(self) -> bool:
        """Poll data from the inverter"""
        pass
    
    def get_data(self) -> InverterData:
        """Get the current inverter data"""
        return self.data

class RegisterMapping(ABC):
    """Abstract base class for register mapping"""
    
    def __init__(self, inverter_type: str):
        self.inverter_type = inverter_type
    
    @abstractmethod
    def get_register_address(self, name: str) -> int:
        """Get the register address for a given name"""
        pass
    
    @abstractmethod
    def get_scaling_factor(self, name: str) -> float:
        """Get the scaling factor for a given name"""
        pass
    
    @abstractmethod
    def get_read_blocks(self) -> List[Dict[str, Any]]:
        """Get the list of register blocks to read"""
        pass
```

### Concrete Implementation for Sol-Ark

The existing Sol-Ark implementation is refactored to inherit from the abstract classes:

```python
from typing import List, Dict, Any
from .base import InverterClient, InverterData, RegisterMapping
from pymodbus.client import ModbusSerialClient

@dataclass
class SolArkData(InverterData):
    """Sol-Ark specific data structure"""
    # Additional Sol-Ark specific fields
    igbt_temp: float = 0.0
    dcdc_xfrmr_temp: float = 0.0
    grid_relay_status: int = 0
    generator_relay_status: int = 0
    inverter_status: int = 0

class SolArkRegisterMapping(RegisterMapping):
    """Sol-Ark register mapping implementation"""
    
    def __init__(self):
        super().__init__("solark")
        self.registers = {
            "grid_voltage_l1l2": 152,
            "grid_voltage_l1n": 150,
            "grid_voltage_l2n": 151,
            "grid_power": 169,
            "battery_soc": 184,
        }
        self.scaling_factors = {
            "grid_voltage_l1l2": 10.0,
            "grid_voltage_l1n": 10.0,
            "grid_voltage_l2n": 10.0,
            "grid_power": 1.0,
            "battery_soc": 1.0,
        }
    
    def get_register_address(self, name: str) -> int:
        return self.registers.get(name, 0)
    
    def get_scaling_factor(self, name: str) -> float:
        return self.scaling_factors.get(name, 1.0)
    
    def get_read_blocks(self) -> List[Dict[str, Any]]:
        return [
            {"start": 150, "count": 20, "description": "Grid data"},
            {"start": 180, "count": 10, "description": "Battery data"},
        ]

class SolArkModbusClient(InverterClient):
    """Sol-Ark Modbus client implementation"""
    
    def __init__(self, port: str, baudrate: int = 9600, modbus_address: int = 1):
        super().__init__(port, baudrate, modbus_address)
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity='N',
            stopbits=1,
            timeout=1.0
        )
        self.register_mapping = SolArkRegisterMapping()
        self.data = SolArkData()
    
    def connect(self) -> bool:
        try:
            return self.client.connect()
        except Exception:
            return False
    
    def disconnect(self) -> None:
        try:
            self.client.close()
        except Exception:
            pass
    
    def poll(self) -> bool:
        try:
            # Read data using register mapping
            blocks = self.register_mapping.get_read_blocks()
            for block in blocks:
                registers = self._read_holding_registers(
                    block["start"], block["count"]
                )
                if registers:
                    self._process_block(block, registers)
            return True
        except Exception:
            return False
```

### Generic Inverter Implementation

A generic implementation allows for configurable register mappings:

```python
import json
from typing import List, Dict, Any, Optional

class GenericRegisterMapping(RegisterMapping):
    """Generic register mapping from JSON configuration"""
    
    def __init__(self, config_file: str):
        super().__init__("generic")
        self.config = self._load_config(config_file)
        self.registers = self.config.get("registers", {})
        self.scaling_factors = self.config.get("scaling_factors", {})
        self.read_blocks = self.config.get("read_blocks", [])
    
    def _load_config(self, config_file: str) -> Dict:
        with open(config_file, 'r') as f:
            return json.load(f)
    
    def get_register_address(self, name: str) -> int:
        reg_info = self.registers.get(name, {})
        return reg_info.get("address", 0)
    
    def get_scaling_factor(self, name: str) -> float:
        reg_info = self.registers.get(name, {})
        return reg_info.get("scaling", 1.0)
    
    def get_read_blocks(self) -> List[Dict[str, Any]]:
        return self.read_blocks

class GenericInverterClient(InverterClient):
    """Generic inverter client using configurable register mapping"""
    
    def __init__(self, port: str, baudrate: int, modbus_address: int, 
                 register_mapping: RegisterMapping):
        super().__init__(port, baudrate, modbus_address)
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity='N',
            stopbits=1,
            timeout=1.0
        )
        self.register_mapping = register_mapping
    
    def connect(self) -> bool:
        try:
            return self.client.connect()
        except Exception:
            return False
    
    def disconnect(self) -> None:
        try:
            self.client.close()
        except Exception:
            pass
    
    def poll(self) -> bool:
        try:
            # Read data using register mapping
            blocks = self.register_mapping.get_read_blocks()
            for block in blocks:
                registers = self._read_holding_registers(
                    block["start"], block["count"]
                )
                if registers:
                    self._process_block(block, registers)
            return True
        except Exception:
            return False
```

### Inverter Factory Pattern

A factory pattern creates the appropriate inverter client:

```python
from typing import Optional

class InverterFactory:
    """Factory for creating inverter clients"""
    
    @staticmethod
    def create_inverter(inverter_type: str, port: str, baudrate: int, 
                       modbus_address: int, **kwargs) -> Optional[InverterClient]:
        if inverter_type == "solark":
            return SolArkModbusClient(port, baudrate, modbus_address)
        elif inverter_type == "generic":
            config_file = kwargs.get("config_file", "generic_registers.json")
            register_mapping = GenericRegisterMapping(config_file)
            return GenericInverterClient(port, baudrate, modbus_address, register_mapping)
        # Add other inverter types as needed
        else:
            return None
```

## Configuration System

### Configuration File Structure

The configuration system supports multiple inverter types:

```yaml
# Generic inverter configuration
inverter:
  type: "solark"  # or "generic", "sma", "fronius", etc.
  modbus_address: 1
  poll_interval: 1.0
  max_retries: 3
  retry_delay: 0.5
  
  # Generic inverter settings
  generic:
    register_mapping_file: "generic_registers.json"
    scaling_factors_file: "generic_scaling.json"
    
serial:
  port: "/dev/ttyRS485"
  baudrate: 9600
  bytesize: 8
  parity: "N"
  stopbits: 1
  timeout: 2.0
```

### Register Configuration Files

Register configuration files define the data points for each inverter type:

```json
{
  "inverter_type": "generic_example",
  "version": "1.0",
  "models": {
    "grid_model": {
      "sunspec_model": 701,
      "instance": 1,
      "context": "grid",
      "description": "Grid-side AC measurements",
      "data_points": {
        "ac_power": {"register": 169, "scaling": 1.0, "sunspec_point": "W"},
        "ac_voltage_l1l2": {"register": 152, "scaling": 10.0, "sunspec_point": "LLV"}
      }
    },
    "load_model": {
      "sunspec_model": 701,
      "instance": 2,
      "context": "load",
      "description": "Load-side AC measurements",
      "data_points": {
        "ac_power": {"register": 178, "scaling": 1.0, "sunspec_point": "W"}
      }
    },
    "battery_model": {
      "sunspec_model": 713,
      "instance": 1,
      "context": "battery",
      "description": "Battery storage capacity",
      "data_points": {
        "battery_soc": {"register": 184, "scaling": 1.0, "sunspec_point": "SoC"},
        "battery_power": {"register": 190, "scaling": 1.0, "sunspec_point": "W"}
      }
    },
    "dc_model": {
      "sunspec_model": 714,
      "instance": 1,
      "context": "dc_ports",
      "description": "DC measurement model with multiple ports",
      "configuration": {
        "num_ports": 5,
        "port_types": ["PV", "PV", "PV", "PV", "ESS"]
      },
      "ports": {
        "pv_port_1": {
          "port_index": 0,
          "port_type": "PV",
          "port_id": 1,
          "port_name": "MPPT1",
          "data_points": {
            "dc_power": {"register": 186, "scaling": 1.0, "sunspec_point": "DCW"}
          }
        },
        "battery_port": {
          "port_index": 4,
          "port_type": "ESS",
          "port_id": 5,
          "port_name": "BATT1",
          "data_points": {
            "dc_power": {"register": 190, "scaling": 1.0, "sunspec_point": "DCW"}
          }
        }
      }
    }
  }
}
```

## SunSpec Model Support

### Multiple Model Instances

The architecture supports multiple instances of SunSpec models:

1. **Model 701 (Inverter)** - Multiple instances for grid and load
2. **Model 713 (Battery)** - Storage capacity monitoring
3. **Model 714 (DC Measurement)** - Multiple DC ports (PV and battery)

### Phase Support Implementation

The system handles different phase configurations:

- 0 = Single-phase
- 1 = Split-phase
- 2 = Three-phase Wye

```python
class PhaseHandler:
    """Handles different phase configurations"""
    
    @staticmethod
    def get_ac_type(grid_type: int) -> int:
        """Convert grid type to SunSpec AC type"""
        # 0=Single Phase, 1=Split Phase, 2=Three Phase Wye
        return grid_type
    
    @staticmethod
    def calculate_phase_values(inverter_data: InverterData):
        """Calculate phase-specific values based on grid type"""
        if inverter_data.grid_type == 0:  # Single-phase
            # Handle single-phase calculations
            pass
        elif inverter_data.grid_type == 1:  # Split-phase
            # Handle split-phase calculations
            pass
        elif inverter_data.grid_type == 2:  # Three-phase
            # Handle three-phase calculations
            pass
```

## Implementation Steps

### Phase 1: Core Abstractions
1. Create abstract base classes for `InverterClient`, `InverterData`, and `RegisterMapping`
2. Refactor `SolArkModbusClient` to inherit from `InverterClient`
3. Create `InverterFactory` class for creating inverter clients

### Phase 2: Register Mapping System
1. Create abstract `RegisterMapping` class
2. Refactor `SolArkRegisterMap` to inherit from `RegisterMapping`
3. Create `GenericRegisterMapping` for configurable register mappings
4. Create JSON schema for defining register mappings

### Phase 3: Configuration System
1. Update `config.yaml` to support generic inverter configuration
2. Modify `EMSApplication` to use the inverter factory
3. Add command-line options for inverter type selection

### Phase 4: Additional Inverter Support
1. Create implementations for other common inverter types (SMA, Fronius, etc.)
2. Create register mapping files for each inverter type
3. Add support for different communication protocols if needed

### Phase 5: Testing and Documentation
1. Create mock implementations for testing
2. Develop test cases for different inverter types
3. Document the new features and usage
4. Create example configurations for different inverter types

## Key Benefits

1. **Extensibility**: Easy to add support for new inverter types
2. **Flexibility**: Configurable register mappings for custom inverters
3. **Compatibility**: Maintains backward compatibility with existing Sol-Ark installations
4. **Standards Compliance**: Proper SunSpec model implementation for all inverter types
5. **Phase Support**: Proper handling of single-phase, split-phase, and three-phase inverters
6. **Multi-Model Support**: Support for multiple instances of SunSpec models (701, 713, 714)

## Example Usage

### Sol-Ark Configuration
```yaml
inverter:
  type: "solark"
  modbus_address: 1
  # Sol-Ark specific settings automatically applied
```

### Generic Inverter Configuration
```yaml
inverter:
  type: "generic"
  modbus_address: 1
  generic:
    register_mapping_file: "my_inverter_registers.json"
```

This architecture provides a clean, extensible way to support multiple inverter types while maintaining backward compatibility with existing Sol-Ark installations.