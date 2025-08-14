"""
Sol-Ark JSON-based implementation of the abstract base classes

All Sol-Ark inverters now use JSON register mapping for consistency and extensibility.
"""

from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field
import time
import logging
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
        self.logger = logging.getLogger(__name__)
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
        
        # Set Sol-Ark specific defaults from JSON config
        phase_config = self.register_mapping.config.get("phase_configuration", {})
        self.data.grid_type = phase_config.get("grid_type", 1)  # Default to split-phase
        self.data.phase_type = phase_config.get("type", "split_phase")
        
        self.logger.debug(f"Sol-Ark configured for grid_type={self.data.grid_type}, phase_type={self.data.phase_type}")
    
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
            
            self.logger.debug(f"Found {len(read_blocks)} read blocks")
            
            for block in read_blocks:
                self.logger.debug(f"Reading block {block['start']}-{block['start'] + block['count'] - 1}")
                registers = self._read_holding_registers(block["start"], block["count"])
                
                if registers is not None:
                    self.logger.debug(f"Successfully read {len(registers)} registers")
                    self._process_json_block(block, registers)
                    success_count += 1
                else:
                    self.logger.warning(f"Failed to read block {block['start']}")
            
            if success_count > 0:
                self.data.last_update = time.time()
                self._calculate_derived_values()
                self.logger.debug(f"Poll successful, updated {success_count} blocks")
                self.logger.debug(f"Sample data - Grid Power: {self.data.grid_power}, Battery SOC: {self.data.battery_soc}")
                return True
            else:
                self.data.last_failure = time.time()
                self.logger.error("Poll failed - no blocks read successfully")
                return False
                
        except Exception as e:
            self.data.last_failure = time.time()
            self.logger.error(f"Poll exception: {e}")
            return False
    
    def _process_json_block(self, block: Dict[str, Any], registers: List[int]):
        """Process data from a JSON-defined read block"""
        try:
            start_register = block["start"]
            self.logger.debug(f"Processing block starting at {start_register} with {len(registers)} registers")
            
            # For each register in the block, find the corresponding data points
            for i, register_value in enumerate(registers):
                register_address = start_register + i
                data_points = self.register_mapping.get_data_points_for_register(register_address)
                
                if data_points:
                    self.logger.debug(f"Register {register_address} = {register_value}, found {len(data_points)} data points")
                
                # Process each data point for this register
                for point in data_points:
                    point_name = point["point_name"]
                    point_config = point["config"]
                    scaling_factor = point_config.get("scaling", 1.0)
                    
                    # Handle signed values first for certain registers
                    if point_name in ["grid_power", "battery_power", "battery_current"]:
                        register_value = self._correct_signed_value(register_value)
                    
                    # Handle temperature registers with offset (Sol-Ark specific)
                    if point_name in ["igbt_temp", "dcdc_xfrmr_temp", "battery_temperature"]:
                        # Temperature registers have an offset of 1000 and scale of 10
                        scaled_value = (register_value - 1000) / 10.0
                    else:
                        # Apply scaling - divide by scaling factor for Sol-Ark (opposite of generic)
                        scaled_value = register_value / scaling_factor if scaling_factor != 0 else register_value
                    
                    self.logger.debug(f"Setting {point_name} = {scaled_value} (raw: {register_value}, scale: {scaling_factor})")
                    
                    # Update the corresponding field in self.data
                    if hasattr(self.data, point_name):
                        setattr(self.data, point_name, scaled_value)
                    else:
                        self.logger.warning(f"Data object has no attribute '{point_name}'")
                        
        except Exception as e:
            self.logger.error(f"Exception in _process_json_block: {e}")
    
    def _calculate_derived_values(self):
        """Calculate derived values from raw measurements"""
        # Calculate total PV power
        self.data.pv_power_total = (self.data.pv1_power + self.data.pv2_power + self.data.pv3_power) / 1000.0
        
        # Set legacy grid_voltage for backward compatibility
        self.data.grid_voltage = self.data.grid_voltage_l1l2
        
        # Ensure phase configuration is maintained (don't override what was set from JSON config)
        if not hasattr(self.data, 'grid_type') or self.data.grid_type == 0:
            phase_config = self.register_mapping.config.get("phase_configuration", {})
            self.data.grid_type = phase_config.get("grid_type", 1)  # Default to split-phase
            self.data.phase_type = phase_config.get("type", "split_phase")
    
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