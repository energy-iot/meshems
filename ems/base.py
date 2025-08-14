"""
Abstract base classes for inverter communication

This module defines the abstract base classes for inverter communication,
data structures, and register mappings. These classes provide the foundation
for supporting multiple inverter types with different register mappings.
"""

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

    # Additional fields for extended data
    inverter_status: int = 0
    battery_temperature: float = 0.0
    igbt_temp: float = 0.0
    dcdc_xfrmr_temp: float = 0.0
    grid_relay_status: int = 0
    generator_relay_status: int = 0
    load_power_total: float = 0.0
    load_power_l1: float = 0.0
    load_power_l2: float = 0.0
    load_current_l1: float = 0.0
    load_current_l2: float = 0.0
    load_frequency: float = 0.0
    inverter_voltage: float = 0.0
    inverter_voltage_ln: float = 0.0
    inverter_voltage_l2n: float = 0.0
    inverter_current_l1: float = 0.0
    inverter_current_l2: float = 0.0
    inverter_frequency: float = 0.0
    inverter_power_l1: float = 0.0
    inverter_power_l2: float = 0.0
    apparent_power: float = 0.0
    grid_power_factor: float = 0.0
    battery_charge_energy: float = 0.0
    battery_discharge_energy: float = 0.0
    grid_buy_energy: float = 0.0
    grid_sell_energy: float = 0.0
    load_energy: float = 0.0
    pv_energy: float = 0.0
    battery_capacity: float = 0.0
    bms_real_time_soc: float = 0.0
    bms_real_time_voltage: float = 0.0
    bms_real_time_current: float = 0.0
    bms_real_time_temp: float = 0.0
    bms_warning: int = 0
    bms_fault: int = 0


class InverterClient(ABC):
    """Abstract base class for inverter communication"""
    
    def __init__(self, port: str, baudrate: int, modbus_address: int):
        """
        Initialize inverter client
        
        Args:
            port: Serial port path
            baudrate: Serial baudrate
            modbus_address: Modbus slave address
        """
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

    def is_grid_connected(self) -> bool:
        """Check if grid is connected"""
        return self.data.grid_relay_status > 0
    
    def is_generator_connected(self) -> bool:
        """Check if generator is connected"""
        return self.data.generator_relay_status > 0
    
    def is_battery_charging(self) -> bool:
        """Check if battery is charging"""
        return self.data.battery_power < 0
    
    def is_battery_discharging(self) -> bool:
        """Check if battery is discharging"""
        return self.data.battery_power > 0
    
    def is_selling_to_grid(self) -> bool:
        """Check if selling power to grid"""
        return self.data.grid_power < 0
    
    def is_buying_from_grid(self) -> bool:
        """Check if buying power from grid"""
        return self.data.grid_power > 0


class RegisterMapping(ABC):
    """Abstract base class for register mapping"""
    
    def __init__(self, inverter_type: str):
        """
        Initialize register mapping
        
        Args:
            inverter_type: Type of inverter this mapping is for
        """
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
        # This method can be overridden by concrete implementations
        pass