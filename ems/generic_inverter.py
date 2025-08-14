"""
Generic inverter client implementation for configurable inverter support
"""

from typing import List, Dict, Any, Optional
from pymodbus.client import ModbusSerialClient
from pymodbus.exceptions import ModbusException
import time

from .base import InverterClient, InverterData
from .generic_mapping import GenericRegisterMapping


class GenericInverterData(InverterData):
    """Generic inverter data structure"""
    
    def __init__(self):
        super().__init__()
        # Add any additional fields specific to generic inverters
        self.comm_version: int = 0
        self.serial_number_parts: List[int] = [0] * 5
        self.corrected_battery_capacity: float = 0.0
        self.battery_empty_voltage: float = 0.0
        self.battery_shutdown_voltage: float = 0.0
        self.battery_restart_voltage: float = 0.0
        self.battery_low_voltage: float = 0.0
        self.battery_shutdown_percent: int = 0
        self.battery_restart_percent: int = 0
        self.battery_low_percent: int = 0
        self.smart_load_power: float = 0.0
        self.last_update: float = time.time()
        self.last_failure: float = 0.0
        self.bms_charging_voltage: float = 0.0
        self.bms_discharge_voltage: float = 0.0
        self.bms_charging_current_limit: float = 0.0
        self.bms_discharge_current_limit: float = 0.0
        self.load_power_l1: float = 0.0
        self.load_power_l2: float = 0.0
        self.inverter_voltage_ln: float = 0.0
        self.inverter_voltage_l2n: float = 0.0
        self.inverter_current_l1: float = 0.0
        self.inverter_current_l2: float = 0.0
        self.inverter_frequency: float = 0.0
        self.inverter_power_l1: float = 0.0
        self.inverter_power_l2: float = 0.0
        self.inverter_output_power: float = 0.0
        self.load_current_l1: float = 0.0
        self.load_current_l2: float = 0.0
        self.load_frequency: float = 0.0
        self.grid_ct_current_l1: float = 0.0
        self.grid_ct_current_l2: float = 0.0
        self.apparent_power: float = 0.0
        self.grid_power_factor: float = 0.0
        self.battery_charge_energy: float = 0.0
        self.battery_discharge_energy: float = 0.0
        self.grid_buy_energy: float = 0.0
        self.grid_sell_energy: float = 0.0
        self.load_energy: float = 0.0
        self.pv_energy: float = 0.0
        self.battery_capacity: float = 0.0
        self.bms_real_time_soc: float = 0.0
        self.bms_real_time_voltage: float = 0.0
        self.bms_real_time_current: float = 0.0
        self.bms_real_time_temp: float = 0.0
        self.bms_warning: int = 0
        self.bms_fault: int = 0
        self.generator_relay_status: int = 0
        self.pv_power_total: float = 0.0
        self.grid_voltage: float = 0.0  # Legacy field


class GenericInverterClient(InverterClient):
    """Generic inverter client using configurable register mapping"""
    
    def __init__(self, port: str, baudrate: int, modbus_address: int, 
                 register_mapping: GenericRegisterMapping):
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
        self.data = GenericInverterData()
    
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
    
    def _process_block(self, block: Dict[str, Any], registers: List[int]):
        """Process data from a read block"""
        try:
            start_register = block["start"]
            
            # For each register in the block, find the corresponding data points
            for i, register_value in enumerate(registers):
                register_address = start_register + i
                data_points = self.register_mapping.get_data_points_for_register(register_address)
                
                # Process each data point for this register
                for point in data_points:
                    point_name = point["point_name"]
                    point_config = point["config"]
                    scaling_factor = point_config.get("scaling", 1.0)
                    
                    # Apply scaling
                    scaled_value = register_value * scaling_factor
                    
                    # Update the corresponding field in self.data
                    if hasattr(self.data, point_name):
                        setattr(self.data, point_name, scaled_value)
                        
        except Exception as e:
            # In a real implementation, we would log this error
            pass