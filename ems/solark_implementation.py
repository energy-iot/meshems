"""
Sol-Ark JSON-based implementation of the abstract base classes

All Sol-Ark inverters now use JSON register mapping for consistency and extensibility.
"""

from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field
import time
from pymodbus.client import ModbusSerialClient
from pymodbus.exceptions import ModbusException

from .base import InverterClient, InverterData, RegisterMapping
from .generic_mapping import GenericRegisterMapping


@dataclass
class SolArkData(InverterData):
    """Sol-Ark specific data structure extending base InverterData"""
    
    # Additional Sol-Ark specific fields
    comm_version: int = 0
    serial_number_parts: List[int] = field(default_factory=lambda: [0] * 5)
    corrected_battery_capacity: float = 0.0
    battery_empty_voltage: float = 0.0
    battery_shutdown_voltage: float = 0.0
    battery_restart_voltage: float = 0.0
    battery_low_voltage: float = 0.0
    battery_shutdown_percent: int = 0
    battery_restart_percent: int = 0
    battery_low_percent: int = 0
    smart_load_power: float = 0.0
    last_update: float = field(default_factory=time.time)
    last_failure: float = 0.0
    bms_charging_voltage: float = 0.0
    bms_discharge_voltage: float = 0.0
    bms_charging_current_limit: float = 0.0
    bms_discharge_current_limit: float = 0.0
    inverter_voltage_ln: float = 0.0
    inverter_voltage_l2n: float = 0.0
    inverter_current_l1: float = 0.0
    inverter_current_l2: float = 0.0
    inverter_frequency: float = 0.0
    inverter_power_l1: float = 0.0
    inverter_power_l2: float = 0.0
    inverter_output_power: float = 0.0
    grid_ct_current_l1: float = 0.0
    grid_ct_current_l2: float = 0.0
    apparent_power: float = 0.0
    battery_charge_energy: float = 0.0
    battery_discharge_energy: float = 0.0
    grid_buy_energy: float = 0.0
    grid_sell_energy: float = 0.0
    pv_energy: float = 0.0
    bms_real_time_soc: float = 0.0
    bms_real_time_voltage: float = 0.0
    bms_real_time_current: float = 0.0
    bms_real_time_temp: float = 0.0
    bms_warning: int = 0
    bms_fault: int = 0
    generator_relay_status: int = 0


class SolArkModbusClient(InverterClient):
    """Sol-Ark Modbus client implementation using JSON register mapping"""
    
    def __init__(self, port: str, baudrate: int = 9600, modbus_address: int = 1,
                 config_file: str = "solark_registers.json"):
        super().__init__(port, baudrate, modbus_address)
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity='N',
            stopbits=1,
            timeout=1.0
        )
        
        # Use JSON-based register mapping exclusively
        self.register_mapping = GenericRegisterMapping(config_file)
        self.data = SolArkData()
        
        # Set Sol-Ark specific defaults
        self.data.phase_type = 'split_phase'  # Sol-Ark is typically split-phase
    
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
        """Poll using JSON-based register mapping"""
        try:
            success_count = 0
            read_blocks = self.register_mapping.get_read_blocks()
            
            for block in read_blocks:
                registers = self._read_holding_registers(block["start"], block["count"])
                
                if registers is not None:
                    self._process_json_block(block, registers)
                    success_count += 1
            
            if success_count > 0:
                self.data.last_update = time.time()
                self._calculate_derived_values()
                return True
            else:
                self.data.last_failure = time.time()
                return False
                
        except Exception:
            self.data.last_failure = time.time()
            return False
    
    def _process_json_block(self, block: Dict[str, Any], registers: List[int]):
        """Process data from a JSON-defined read block"""
        try:
            # Get register definitions from the block
            block_registers = block.get("registers", [])
            
            for reg_def in block_registers:
                reg_address = reg_def["address"]
                reg_name = reg_def["name"]
                
                # Calculate offset within the block
                offset = reg_address - block["start"]
                if 0 <= offset < len(registers):
                    raw_value = registers[offset]
                    
                    # Apply scaling factor
                    scaling = self.register_mapping.get_scaling_factor(reg_name)
                    scaled_value = raw_value / scaling if scaling != 0 else raw_value
                    
                    # Handle signed values for certain registers
                    if reg_name in ["grid_power", "battery_power", "battery_current", 
                                   "inverter_output_power", "inverter_power_l1", "inverter_power_l2"]:
                        scaled_value = self._correct_signed_value(raw_value) / scaling if scaling != 0 else self._correct_signed_value(raw_value)
                    
                    # Handle temperature registers with offset
                    if reg_name in ["igbt_temp", "dcdc_xfrmr_temp", "battery_temperature", "bms_real_time_temp"]:
                        # Temperature registers have an offset of 1000 and scale of 10
                        scaled_value = (raw_value - 1000) / 10.0
                    
                    # Set the value in the data object
                    if hasattr(self.data, reg_name):
                        setattr(self.data, reg_name, scaled_value)
                        
        except Exception as e:
            pass  # In a real implementation, we would log this error
    
    def _calculate_derived_values(self):
        """Calculate derived values from raw measurements"""
        # Calculate total PV power
        self.data.pv_power_total = (self.data.pv1_power + self.data.pv2_power + self.data.pv3_power) / 1000.0
        
        # Set legacy grid_voltage for backward compatibility
        self.data.grid_voltage = self.data.grid_voltage_l1l2
        
        # Ensure phase type is set
        self.data.phase_type = 'split_phase'  # Sol-Ark is typically split-phase
    
    def _correct_signed_value(self, value: int) -> int:
        """Convert unsigned 16-bit value to signed if necessary"""
        if value > 32767:
            return value - 65536
        return value
    
    def _read_holding_registers(self, start_register: int, num_registers: int) -> Optional[List[int]]:
        """Read holding registers from the device"""
        try:
            result = self.client.read_holding_registers(
                address=start_register,
                count=num_registers,
                device_id=self.modbus_address
            )
            
            if result.isError():
                return None
            
            return result.registers
        
        except ModbusException:
            return None
        except Exception:
            return None
    
    def get_data(self) -> InverterData:
        """Get the current inverter data"""
        return self.data
    
    # Convenience methods for status checking
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
    
    def get_serial_number(self) -> str:
        """Get formatted serial number string"""
        serial_str = ""
        for part in self.data.serial_number_parts:
            if part == 0:
                break
            char1 = (part >> 8) & 0xFF
            char2 = part & 0xFF
            if char1 != 0:
                serial_str += chr(char1)
            if char2 != 0:
                serial_str += chr(char2)
        return serial_str
    
    def get_battery_temperature_f(self) -> float:
        """Get battery temperature in Fahrenheit"""
        return (self.data.battery_temperature * 9.0 / 5.0) + 32.0