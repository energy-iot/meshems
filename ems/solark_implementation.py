"""
Sol-Ark specific implementation of the abstract base classes
"""

from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field
import time
from pymodbus.client import ModbusSerialClient
from pymodbus.exceptions import ModbusException

from .base import InverterClient, InverterData, RegisterMapping
from .solark_registers import SolArkRegisterMap, SolArkScalingFactors, SolArkBlockType, ModbusReadBlock


@dataclass
class SolArkData(InverterData):
    """Sol-Ark specific data structure"""
    
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
    load_power_l1: float = 0.0
    load_power_l2: float = 0.0
    inverter_voltage_ln: float = 0.0
    inverter_voltage_l2n: float = 0.0
    inverter_current_l1: float = 0.0
    inverter_current_l2: float = 0.0
    inverter_frequency: float = 0.0
    inverter_power_l1: float = 0.0
    inverter_power_l2: float = 0.0
    inverter_output_power: float = 0.0
    load_current_l1: float = 0.0
    load_current_l2: float = 0.0
    load_frequency: float = 0.0
    grid_ct_current_l1: float = 0.0
    grid_ct_current_l2: float = 0.0
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
    generator_relay_status: int = 0
    pv_power_total: float = 0.0
    grid_voltage: float = 0.0  # Legacy field


class SolArkRegisterMapping(RegisterMapping):
    """Sol-Ark register mapping implementation"""
    
    def __init__(self):
        super().__init__("solark")
        self.registers = {
            "grid_voltage_l1l2": SolArkRegisterMap.GRID_VOLTAGE_L1L2,
            "grid_voltage_l1n": SolArkRegisterMap.GRID_VOLTAGE_L1N,
            "grid_voltage_l2n": SolArkRegisterMap.GRID_VOLTAGE_L2N,
            "grid_power": SolArkRegisterMap.GRID_POWER,
            "battery_soc": SolArkRegisterMap.BATTERY_SOC,
            "battery_voltage": SolArkRegisterMap.BATTERY_VOLTAGE,
            "battery_current": SolArkRegisterMap.BATTERY_CURRENT,
            "battery_power": SolArkRegisterMap.BATTERY_POWER,
            "pv1_power": SolArkRegisterMap.PV1_POWER,
            "pv2_power": SolArkRegisterMap.PV2_POWER,
            "grid_current_l1": SolArkRegisterMap.GRID_CURRENT_L1,
            "grid_current_l2": SolArkRegisterMap.GRID_CURRENT_L2,
            "grid_frequency": SolArkRegisterMap.GRID_FREQUENCY,
            "inverter_status": SolArkRegisterMap.INVERTER_STATUS,
            "igbt_temp": SolArkRegisterMap.IGBT_HEATSINK_TEMP,
            "dcdc_xfrmr_temp": SolArkRegisterMap.DCDC_XFRMR_TEMP,
            "grid_relay_status": SolArkRegisterMap.GRID_RELAY_STATUS,
            "generator_relay_status": SolArkRegisterMap.GENERATOR_RELAY_STATUS,
            "load_power_total": SolArkRegisterMap.LOAD_POWER_TOTAL,
            "load_power_l1": SolArkRegisterMap.LOAD_POWER_L1,
            "load_power_l2": SolArkRegisterMap.LOAD_POWER_L2,
            "load_current_l1": SolArkRegisterMap.LOAD_CURRENT_L1,
            "load_current_l2": SolArkRegisterMap.LOAD_CURRENT_L2,
            "load_frequency": SolArkRegisterMap.LOAD_FREQUENCY,
            "inverter_voltage": SolArkRegisterMap.INVERTER_VOLTAGE,
            "inverter_voltage_ln": SolArkRegisterMap.INVERTER_VOLTAGE_LN,
            "inverter_voltage_l2n": SolArkRegisterMap.INVERTER_VOLTAGE_L2N,
            "inverter_current_l1": SolArkRegisterMap.INVERTER_CURRENT_L1,
            "inverter_current_l2": SolArkRegisterMap.INVERTER_CURRENT_L2,
            "inverter_frequency": SolArkRegisterMap.INVERTER_FREQUENCY,
            "inverter_power_l1": SolArkRegisterMap.INVERTER_POWER_L1,
            "inverter_power_l2": SolArkRegisterMap.INVERTER_POWER_L2,
            "apparent_power": SolArkRegisterMap.APPARENT_POWER,
            "grid_power_factor": SolArkRegisterMap.GRID_POWER_FACTOR,
            "battery_charge_energy": SolArkRegisterMap.BATTERY_CHARGE_ENERGY,
            "battery_discharge_energy": SolArkRegisterMap.BATTERY_DISCHARGE_ENERGY,
            "grid_buy_energy": SolArkRegisterMap.GRID_BUY_ENERGY,
            "grid_sell_energy": SolArkRegisterMap.GRID_SELL_ENERGY,
            "load_energy": SolArkRegisterMap.LOAD_ENERGY,
            "pv_energy": SolArkRegisterMap.PV_ENERGY,
            "battery_capacity": SolArkRegisterMap.BATTERY_CAPACITY,
            "bms_real_time_soc": SolArkRegisterMap.BMS_REAL_TIME_SOC,
            "bms_real_time_voltage": SolArkRegisterMap.BMS_REAL_TIME_VOLTAGE,
            "bms_real_time_current": SolArkRegisterMap.BMS_REAL_TIME_CURRENT,
            "bms_real_time_temp": SolArkRegisterMap.BMS_REAL_TIME_TEMP,
            "bms_warning": SolArkRegisterMap.BMS_WARNING,
            "bms_fault": SolArkRegisterMap.BMS_FAULT,
            "grid_type": SolArkRegisterMap.GRID_TYPE,
            "comm_version": SolArkRegisterMap.COMM_VERSION,
            "smart_load_power": SolArkRegisterMap.SMART_LOAD_POWER,
            "battery_temperature": SolArkRegisterMap.BATTERY_TEMPERATURE,
        }
        self.scaling_factors = {
            "grid_voltage_l1l2": SolArkScalingFactors.VOLTAGE,
            "grid_voltage_l1n": SolArkScalingFactors.VOLTAGE,
            "grid_voltage_l2n": SolArkScalingFactors.VOLTAGE,
            "grid_power": 1.0,
            "battery_soc": 1.0,
            "battery_voltage": SolArkScalingFactors.VOLTAGE,
            "battery_current": SolArkScalingFactors.CURRENT,
            "battery_power": 1.0,
            "pv1_power": 1.0,
            "pv2_power": 1.0,
            "grid_current_l1": SolArkScalingFactors.CURRENT,
            "grid_current_l2": SolArkScalingFactors.CURRENT,
            "grid_frequency": SolArkScalingFactors.FREQUENCY,
            "inverter_status": 1.0,
            "igbt_temp": SolArkScalingFactors.TEMPERATURE_SCALE,
            "dcdc_xfrmr_temp": SolArkScalingFactors.TEMPERATURE_SCALE,
            "grid_relay_status": 1.0,
            "generator_relay_status": 1.0,
            "load_power_total": 1.0,
            "load_power_l1": 1.0,
            "load_power_l2": 1.0,
            "load_current_l1": SolArkScalingFactors.CURRENT,
            "load_current_l2": SolArkScalingFactors.CURRENT,
            "load_frequency": SolArkScalingFactors.FREQUENCY,
            "inverter_voltage": SolArkScalingFactors.VOLTAGE,
            "inverter_voltage_ln": SolArkScalingFactors.VOLTAGE,
            "inverter_voltage_l2n": SolArkScalingFactors.VOLTAGE,
            "inverter_current_l1": SolArkScalingFactors.CURRENT,
            "inverter_current_l2": SolArkScalingFactors.CURRENT,
            "inverter_frequency": SolArkScalingFactors.FREQUENCY,
            "inverter_power_l1": 1.0,
            "inverter_power_l2": 1.0,
            "apparent_power": 1.0,
            "grid_power_factor": 1.0,
            "battery_charge_energy": SolArkScalingFactors.ENERGY,
            "battery_discharge_energy": SolArkScalingFactors.ENERGY,
            "grid_buy_energy": SolArkScalingFactors.ENERGY,
            "grid_sell_energy": SolArkScalingFactors.ENERGY,
            "load_energy": SolArkScalingFactors.ENERGY,
            "pv_energy": SolArkScalingFactors.ENERGY,
            "battery_capacity": 1.0,
            "bms_real_time_soc": 1.0,
            "bms_real_time_voltage": SolArkScalingFactors.VOLTAGE,
            "bms_real_time_current": 1.0,
            "bms_real_time_temp": SolArkScalingFactors.TEMPERATURE_SCALE,
            "bms_warning": 1.0,
            "bms_fault": 1.0,
            "grid_type": 1.0,
            "comm_version": 1.0,
            "smart_load_power": 1.0,
            "battery_temperature": SolArkScalingFactors.TEMPERATURE_SCALE,
        }
    
    def get_register_address(self, name: str) -> int:
        return self.registers.get(name, 0)
    
    def get_scaling_factor(self, name: str) -> float:
        return self.scaling_factors.get(name, 1.0)
    
    def get_read_blocks(self) -> List[Dict[str, Any]]:
        # Return the existing SOLARK_READ_BLOCKS from solark_registers.py
        from .solark_registers import SOLARK_READ_BLOCKS
        return [{"start": block.start_register, "count": block.num_registers, "description": block.description} 
                for block in SOLARK_READ_BLOCKS]


class SolArkModbusClient(InverterClient):
    """Sol-Ark Modbus client implementation"""
    
    def __init__(self, port: str, baudrate: int = 9600, modbus_address: int = 1,
                 use_json_mapping: bool = False, config_file: str = "solark_registers.json"):
        super().__init__(port, baudrate, modbus_address)
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity='N',
            stopbits=1,
            timeout=1.0
        )
        
        # Choose mapping strategy based on configuration
        self.use_json_mapping = use_json_mapping
        if use_json_mapping:
            from .generic_mapping import GenericRegisterMapping
            self.register_mapping = GenericRegisterMapping(config_file)
        else:
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
            if self.use_json_mapping:
                # Use JSON-based polling (similar to generic implementation)
                return self._poll_json_mapping()
            else:
                # Use legacy hardcoded polling
                return self._poll_legacy_mapping()
                
        except Exception:
            self.data.last_failure = time.time()
            return False
    
    def _poll_json_mapping(self) -> bool:
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
                return True
            else:
                self.data.last_failure = time.time()
                return False
                
        except Exception:
            self.data.last_failure = time.time()
            return False
    
    def _poll_legacy_mapping(self) -> bool:
        """Poll using legacy hardcoded register mapping"""
        try:
            # Import SOLARK_READ_BLOCKS here to avoid circular imports
            from .solark_registers import SOLARK_READ_BLOCKS, SolArkRegisterMap
            
            success_count = 0
            total_blocks = len(SOLARK_READ_BLOCKS)
            
            for block in SOLARK_READ_BLOCKS:
                registers = self._read_holding_registers(block.start_register, block.num_registers)
                
                if registers is not None:
                    self._process_block(block, registers)
                    success_count += 1
            
            if success_count > 0:
                self.data.last_update = time.time()
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
                    if reg_name in ["grid_power", "battery_power", "battery_current"]:
                        scaled_value = self._correct_signed_value(raw_value) / scaling if scaling != 0 else self._correct_signed_value(raw_value)
                    
                    # Set the value in the data object
                    if hasattr(self.data, reg_name):
                        setattr(self.data, reg_name, scaled_value)
            
            # Calculate derived values
            self._calculate_derived_values()
                        
        except Exception as e:
            pass  # In a real implementation, we would log this error
    
    def _calculate_derived_values(self):
        """Calculate derived values from raw measurements"""
        # Calculate total PV power
        self.data.pv_power_total = (self.data.pv1_power + self.data.pv2_power + self.data.pv3_power) / 1000.0
        
        # Set legacy grid_voltage for backward compatibility
        self.data.grid_voltage = self.data.grid_voltage_l1l2
        
        # Set phase type for SunSpec
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
    
    def _process_block(self, block: ModbusReadBlock, registers: List[int]):
        """Process data from a read block"""
        try:
            from .solark_registers import SOLARK_READ_BLOCKS, SolArkRegisterMap, SolArkScalingFactors
            
            if block.block_type == SolArkBlockType.ENERGY:
                # Registers 70-84 (15 regs)
                offset = SolArkRegisterMap.BATTERY_CHARGE_ENERGY - block.start_register
                self.data.battery_charge_energy = registers[offset] / SolArkScalingFactors.ENERGY
                
                offset = SolArkRegisterMap.BATTERY_DISCHARGE_ENERGY - block.start_register
                self.data.battery_discharge_energy = registers[offset] / SolArkScalingFactors.ENERGY
                
                offset = SolArkRegisterMap.GRID_BUY_ENERGY - block.start_register
                self.data.grid_buy_energy = registers[offset] / SolArkScalingFactors.ENERGY
                
                offset = SolArkRegisterMap.GRID_SELL_ENERGY - block.start_register
                self.data.grid_sell_energy = registers[offset] / SolArkScalingFactors.ENERGY
                
                offset = SolArkRegisterMap.GRID_FREQUENCY - block.start_register
                self.data.grid_frequency = registers[offset] / SolArkScalingFactors.FREQUENCY
                
                offset = SolArkRegisterMap.LOAD_ENERGY - block.start_register
                self.data.load_energy = registers[offset] / SolArkScalingFactors.ENERGY
            
            elif block.block_type == SolArkBlockType.PV_ENERGY:
                # Register 108 (1 reg)
                self.data.pv_energy = registers[0] / SolArkScalingFactors.ENERGY
            
            elif block.block_type == SolArkBlockType.INVERTER_STATUS:
                # Register 59 (1 reg)
                self.data.inverter_status = registers[0]
            
            elif block.block_type == SolArkBlockType.TEMPERATURES:
                # Registers 90-91 (2 regs)
                offset = SolArkRegisterMap.DCDC_XFRMR_TEMP - block.start_register
                self.data.dcdc_xfrmr_temp = (registers[offset] - SolArkScalingFactors.TEMPERATURE_OFFSET) / SolArkScalingFactors.TEMPERATURE_SCALE
                
                offset = SolArkRegisterMap.IGBT_HEATSINK_TEMP - block.start_register
                self.data.igbt_temp = (registers[offset] - SolArkScalingFactors.TEMPERATURE_OFFSET) / SolArkScalingFactors.TEMPERATURE_SCALE
            
            elif block.block_type == SolArkBlockType.APPARENT_POWER_38:
                # Register 38 (1 reg)
                self.data.apparent_power = registers[0]
            
            elif block.block_type == SolArkBlockType.GRID_POWER_FACTOR_89:
                # Register 89 (1 reg)
                self.data.grid_power_factor = registers[0] / SolArkScalingFactors.CURRENT  # Power factor is scaled by 100
            
            elif block.block_type == SolArkBlockType.GRID_INVERTER_150:
                # Registers 150-169 (20 regs)
                # Grid voltage registers (new specific registers)
                offset = SolArkRegisterMap.GRID_VOLTAGE_L1N - block.start_register
                self.data.grid_voltage_l1n = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                offset = SolArkRegisterMap.GRID_VOLTAGE_L2N - block.start_register
                self.data.grid_voltage_l2n = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                offset = SolArkRegisterMap.GRID_VOLTAGE_L1L2 - block.start_register
                self.data.grid_voltage_l1l2 = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                # Legacy grid_voltage for backward compatibility - use L1L2 voltage
                self.data.grid_voltage = self.data.grid_voltage_l1l2
                
                # Inverter voltage registers
                offset = SolArkRegisterMap.INVERTER_VOLTAGE_LN - block.start_register
                self.data.inverter_voltage_ln = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                offset = SolArkRegisterMap.INVERTER_VOLTAGE_L2N - block.start_register
                self.data.inverter_voltage_l2n = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                offset = SolArkRegisterMap.INVERTER_VOLTAGE - block.start_register
                self.data.inverter_voltage = registers[offset] / SolArkScalingFactors.VOLTAGE
                
                offset = SolArkRegisterMap.GRID_CURRENT_L1 - block.start_register
                self.data.grid_current_l1 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.GRID_CURRENT_L2 - block.start_register
                self.data.grid_current_l2 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.GRID_CT_CURRENT_L1 - block.start_register
                self.data.grid_ct_current_l1 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.GRID_CT_CURRENT_L2 - block.start_register
                self.data.grid_ct_current_l2 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.INVERTER_CURRENT_L1 - block.start_register
                self.data.inverter_current_l1 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.INVERTER_CURRENT_L2 - block.start_register
                self.data.inverter_current_l2 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.SMART_LOAD_POWER - block.start_register
                self.data.smart_load_power = registers[offset]
                
                offset = SolArkRegisterMap.GRID_POWER - block.start_register
                self.data.grid_power = self._correct_signed_value(registers[offset])
            
            elif block.block_type == SolArkBlockType.POWER_BATTERY_170:
                # Registers 170-189 (20 regs)
                offset = SolArkRegisterMap.INVERTER_OUTPUT_POWER - block.start_register
                self.data.inverter_output_power = self._correct_signed_value(registers[offset])
                
                offset = SolArkRegisterMap.LOAD_POWER_L1 - block.start_register
                self.data.load_power_l1 = registers[offset]
                
                offset = SolArkRegisterMap.LOAD_POWER_L2 - block.start_register
                self.data.load_power_l2 = registers[offset]
                
                offset = SolArkRegisterMap.LOAD_POWER_TOTAL - block.start_register
                self.data.load_power_total = registers[offset]
                
                offset = SolArkRegisterMap.LOAD_CURRENT_L1 - block.start_register
                self.data.load_current_l1 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.LOAD_CURRENT_L2 - block.start_register
                self.data.load_current_l2 = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BATTERY_TEMPERATURE - block.start_register
                self.data.battery_temperature = (registers[offset] - SolArkScalingFactors.TEMPERATURE_OFFSET) / SolArkScalingFactors.TEMPERATURE_SCALE
                
                offset = SolArkRegisterMap.BATTERY_VOLTAGE - block.start_register
                self.data.battery_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BATTERY_SOC - block.start_register
                self.data.battery_soc = registers[offset]
                
                offset = SolArkRegisterMap.PV1_POWER - block.start_register
                self.data.pv1_power = registers[offset]
                
                offset = SolArkRegisterMap.PV2_POWER - block.start_register
                self.data.pv2_power = registers[offset]
                
                # Process inverter power L1 and L2 (WL1, WL2)
                offset = SolArkRegisterMap.INVERTER_POWER_L1 - block.start_register
                self.data.inverter_power_l1 = registers[offset]
                
                offset = SolArkRegisterMap.INVERTER_POWER_L2 - block.start_register
                self.data.inverter_power_l2 = registers[offset]
                
                self.data.pv_power_total = (self.data.pv1_power + self.data.pv2_power) / 1000.0
            
            elif block.block_type == SolArkBlockType.BATTERY_STATUS_190:
                # Registers 190-199 (10 regs)
                offset = SolArkRegisterMap.BATTERY_POWER - block.start_register
                self.data.battery_power = self._correct_signed_value(registers[offset])
                
                offset = SolArkRegisterMap.BATTERY_CURRENT - block.start_register
                self.data.battery_current = self._correct_signed_value(registers[offset]) / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.LOAD_FREQUENCY - block.start_register
                self.data.load_frequency = registers[offset] / SolArkScalingFactors.FREQUENCY
                
                offset = SolArkRegisterMap.INVERTER_FREQUENCY - block.start_register
                self.data.inverter_frequency = registers[offset] / SolArkScalingFactors.FREQUENCY
                
                offset = SolArkRegisterMap.GRID_RELAY_STATUS - block.start_register
                self.data.grid_relay_status = registers[offset]
                
                offset = SolArkRegisterMap.GENERATOR_RELAY_STATUS - block.start_register
                self.data.generator_relay_status = registers[offset]
            
            elif block.block_type == SolArkBlockType.BATTERY_CAPACITY_204:
                # Register 204
                self.data.battery_capacity = registers[0]
            
            elif block.block_type == SolArkBlockType.CORRECTED_BATTERY_CAPACITY_107:
                # Register 107
                self.data.corrected_battery_capacity = registers[0]
            
            elif block.block_type == SolArkBlockType.BATTERY_EMPTY_VOLTAGE_205:
                # Register 205
                self.data.battery_empty_voltage = registers[0] / SolArkScalingFactors.CURRENT
            
            elif block.block_type == SolArkBlockType.BATTERY_VOLTAGE_THRESHOLDS_220:
                # Registers 220-222 (3 regs)
                offset = SolArkRegisterMap.BATTERY_SHUTDOWN_VOLTAGE - block.start_register
                self.data.battery_shutdown_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BATTERY_RESTART_VOLTAGE - block.start_register
                self.data.battery_restart_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BATTERY_LOW_VOLTAGE - block.start_register
                self.data.battery_low_voltage = registers[offset] / SolArkScalingFactors.CURRENT
            
            elif block.block_type == SolArkBlockType.BATTERY_PERCENT_THRESHOLDS_217:
                # Registers 217-219 (3 regs)
                offset = SolArkRegisterMap.BATTERY_SHUTDOWN_PERCENT - block.start_register
                self.data.battery_shutdown_percent = registers[offset]
                
                offset = SolArkRegisterMap.BATTERY_RESTART_PERCENT - block.start_register
                self.data.battery_restart_percent = registers[offset]
                
                offset = SolArkRegisterMap.BATTERY_LOW_PERCENT - block.start_register
                self.data.battery_low_percent = registers[offset]
            
            elif block.block_type == SolArkBlockType.BMS_DATA_312:
                # Registers 312-323 (12 regs)
                offset = SolArkRegisterMap.BMS_CHARGING_VOLTAGE - block.start_register
                self.data.bms_charging_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BMS_DISCHARGE_VOLTAGE - block.start_register
                self.data.bms_discharge_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BMS_CHARGING_CURRENT_LIMIT - block.start_register
                self.data.bms_charging_current_limit = registers[offset]
                
                offset = SolArkRegisterMap.BMS_DISCHARGE_CURRENT_LIMIT - block.start_register
                self.data.bms_discharge_current_limit = registers[offset]
                
                offset = SolArkRegisterMap.BMS_REAL_TIME_SOC - block.start_register
                self.data.bms_real_time_soc = registers[offset]
                
                offset = SolArkRegisterMap.BMS_REAL_TIME_VOLTAGE - block.start_register
                self.data.bms_real_time_voltage = registers[offset] / SolArkScalingFactors.CURRENT
                
                offset = SolArkRegisterMap.BMS_REAL_TIME_CURRENT - block.start_register
                self.data.bms_real_time_current = registers[offset]
                
                offset = SolArkRegisterMap.BMS_REAL_TIME_TEMP - block.start_register
                self.data.bms_real_time_temp = (registers[offset] - SolArkScalingFactors.TEMPERATURE_OFFSET) / SolArkScalingFactors.TEMPERATURE_SCALE
                
                offset = SolArkRegisterMap.BMS_WARNING - block.start_register
                self.data.bms_warning = registers[offset]
                
                offset = SolArkRegisterMap.BMS_FAULT - block.start_register
                self.data.bms_fault = registers[offset]
            
            elif block.block_type == SolArkBlockType.GRID_TYPE_286:
                # Register 286
                self.data.grid_type = registers[0]
            
            elif block.block_type == SolArkBlockType.DIAGNOSTICS:
                # Registers 2-7
                offset = SolArkRegisterMap.COMM_VERSION - block.start_register
                self.data.comm_version = registers[offset]
                
                for i in range(5):
                    offset = (SolArkRegisterMap.SN_BYTE_01 + i) - block.start_register
                    if offset < len(registers):
                        self.data.serial_number_parts[i] = registers[offset]
            
            else:
                pass  # Unknown block type, ignore
                
        except Exception as e:
            pass  # In a real implementation, we would log this error