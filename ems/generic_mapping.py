"""
Generic register mapping system for configurable inverter support
"""

import json
import os
from typing import List, Dict, Any, Optional
from .base import RegisterMapping


class GenericRegisterMapping(RegisterMapping):
    """Generic register mapping from JSON configuration"""
    
    def __init__(self, config_file: str):
        super().__init__("generic")
        self.config = self._load_config(config_file)
        self.models = self.config.get("models", {})
        
        print(f"DEBUG: Loaded config from {config_file}")
        print(f"DEBUG: Found {len(self.models)} models: {list(self.models.keys())}")
        
        # Build registers and scaling factors from the JSON structure
        self.registers = {}
        self.scaling_factors = {}
        self._build_register_mappings()
        
        print(f"DEBUG: Built {len(self.registers)} register mappings")
        print(f"DEBUG: Sample registers: {dict(list(self.registers.items())[:5])}")
        
        # Build read blocks from the data points
        self.read_blocks = self._build_read_blocks()
        print(f"DEBUG: Built {len(self.read_blocks)} read blocks")
        for block in self.read_blocks:
            print(f"DEBUG: Block {block['start']}-{block['start'] + block['count'] - 1}: {block['description']}")
    
    def _load_config(self, config_file: str) -> Dict:
        # Handle relative paths
        if not os.path.isabs(config_file):
            config_file = os.path.join(os.path.dirname(__file__), config_file)
        
        with open(config_file, 'r') as f:
            return json.load(f)
    
    def _build_register_mappings(self):
        """Build register mappings from the JSON structure"""
        for model_name, model_config in self.models.items():
            data_points = model_config.get("data_points", {})
            for point_name, point_config in data_points.items():
                register = point_config.get("register", 0)
                scaling = point_config.get("scaling", 1.0)
                
                # Store register address and scaling factor
                self.registers[point_name] = register
                self.scaling_factors[point_name] = scaling
    
    def _build_read_blocks(self) -> List[Dict[str, Any]]:
        """Build read blocks from the data points"""
        # Collect all unique registers and group them into blocks
        registers = []
        for model_name, model_config in self.models.items():
            data_points = model_config.get("data_points", {})
            for point_name, point_config in data_points.items():
                register = point_config.get("register", 0)
                registers.append((register, point_name))
        
        # Sort registers by address
        registers.sort()
        
        # Group registers into blocks (max 20 registers per block)
        blocks = []
        if registers:
            current_start = registers[0][0]
            current_end = current_start
            current_registers = [registers[0]]
            
            for reg_addr, reg_name in registers[1:]:
                # If this register is within 20 of the current block start, add it
                if reg_addr <= current_start + 20:
                    current_end = reg_addr
                    current_registers.append((reg_addr, reg_name))
                else:
                    # Create a block with the current registers
                    if current_registers:
                        blocks.append({
                            "start": current_start,
                            "count": current_end - current_start + 1,
                            "description": f"Registers {current_start}-{current_end}"
                        })
                    
                    # Start a new block
                    current_start = reg_addr
                    current_end = reg_addr
                    current_registers = [(reg_addr, reg_name)]
            
            # Add the last block
            if current_registers:
                blocks.append({
                    "start": current_start,
                    "count": current_end - current_start + 1,
                    "description": f"Registers {current_start}-{current_end}"
                })
        
        return blocks
    
    def get_register_address(self, name: str) -> int:
        return self.registers.get(name, 0)
    
    def get_scaling_factor(self, name: str) -> float:
        return self.scaling_factors.get(name, 1.0)
    
    def get_read_blocks(self) -> List[Dict[str, Any]]:
        return self.read_blocks
    
    def get_data_points_for_register(self, register_address: int) -> List[Dict[str, Any]]:
        """Get all data points that correspond to a specific register"""
        data_points = []
        for model_name, model_config in self.models.items():
            data_points_config = model_config.get("data_points", {})
            for point_name, point_config in data_points_config.items():
                if point_config.get("register") == register_address:
                    data_points.append({
                        "model": model_name,
                        "point_name": point_name,
                        "config": point_config
                    })
        return data_points


class GenericInverterData:
    """Generic inverter data structure"""
    
    def __init__(self):
        # Initialize with default values
        self.grid_voltage_l1l2 = 0.0
        self.grid_voltage_l1n = 0.0
        self.grid_voltage_l2n = 0.0
        self.grid_power = 0.0
        self.battery_soc = 0.0
        self.battery_voltage = 0.0
        self.battery_current = 0.0
        self.battery_power = 0.0
        self.pv1_power = 0.0
        self.pv2_power = 0.0
        self.grid_current_l1 = 0.0
        self.grid_current_l2 = 0.0
        self.grid_frequency = 0.0
        self.inverter_status = 0
        self.igbt_temp = 0.0
        self.dcdc_xfrmr_temp = 0.0
        self.grid_relay_status = 0
        self.generator_relay_status = 0
        self.load_power_total = 0.0
        self.load_power_l1 = 0.0
        self.load_power_l2 = 0.0
        self.load_current_l1 = 0.0
        self.load_current_l2 = 0.0
        self.load_frequency = 0.0
        self.inverter_voltage = 0.0
        self.inverter_voltage_ln = 0.0
        self.inverter_voltage_l2n = 0.0
        self.inverter_current_l1 = 0.0
        self.inverter_current_l2 = 0.0
        self.inverter_frequency = 0.0
        self.inverter_power_l1 = 0.0
        self.inverter_power_l2 = 0.0
        self.apparent_power = 0.0
        self.grid_power_factor = 0.0
        self.battery_charge_energy = 0.0
        self.battery_discharge_energy = 0.0
        self.grid_buy_energy = 0.0
        self.grid_sell_energy = 0.0
        self.load_energy = 0.0
        self.pv_energy = 0.0
        self.battery_capacity = 0.0
        self.bms_real_time_soc = 0.0
        self.bms_real_time_voltage = 0.0
        self.bms_real_time_current = 0.0
        self.bms_real_time_temp = 0.0
        self.bms_warning = 0
        self.bms_fault = 0
        self.grid_type = 0
        self.comm_version = 0
        self.smart_load_power = 0.0
        self.battery_temperature = 0.0
        self.pv_power_total = 0.0
        self.grid_voltage = 0.0  # Legacy field
        self.grid_ct_current_l1 = 0.0
        self.grid_ct_current_l2 = 0.0
        self.inverter_output_power = 0.0
        self.corrected_battery_capacity = 0.0
        self.battery_empty_voltage = 0.0
        self.battery_shutdown_voltage = 0.0
        self.battery_restart_voltage = 0.0
        self.battery_low_voltage = 0.0
        self.battery_shutdown_percent = 0
        self.battery_restart_percent = 0
        self.battery_low_percent = 0
        self.bms_charging_voltage = 0.0
        self.bms_discharge_voltage = 0.0
        self.bms_charging_current_limit = 0.0
        self.bms_discharge_current_limit = 0.0
        self.serial_number_parts = [0] * 5
        self.last_update = 0.0
        self.last_failure = 0.0


class GenericInverterClient:
    """Generic inverter client using configurable register mapping"""
    
    def __init__(self, port: str, baudrate: int, modbus_address: int, 
                 register_mapping: GenericRegisterMapping):
        self.port = port
        self.baudrate = baudrate
        self.modbus_address = modbus_address
        self.register_mapping = register_mapping
        self.data = GenericInverterData()
        
        # Initialize Modbus client
        from pymodbus.client import ModbusSerialClient
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity='N',
            stopbits=1,
            timeout=1.0
        )
    
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
            from pymodbus.exceptions import ModbusException
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
                    
                    # Apply scaling and update the data object
                    scaled_value = register_value * scaling_factor
                    
                    # Update the corresponding field in self.data
                    if hasattr(self.data, point_name):
                        setattr(self.data, point_name, scaled_value)
                        
        except Exception as e:
            pass  # In a real implementation, we would log this error