"""
SunSpec Models Implementation

This module implements SunSpec-compliant data models for exposing Sol-Ark inverter data
in a standardized format over Modbus TCP.
"""

import logging
import time
from typing import Dict, Any, Optional
from dataclasses import dataclass, field

from .solark_client import SolArkData


@dataclass
class SunSpecCommonModel:
    """SunSpec Common Model (Model 1) - Device identification"""
    
    # Fixed header
    sunspec_id: int = 0x53756E53  # 'SunS' in ASCII
    model_id: int = 1
    model_length: int = 65
    
    # Device information
    manufacturer: str = "Energy IoT Open Source"
    model: str = "EMS-Dev Python"
    options: str = "Sol-Ark Gateway"
    version: str = "1.0.0"
    serial_number: str = "EMS-PY-001"
    device_address: int = 1


@dataclass
class SunSpecInverterModel:
    """SunSpec Inverter Model (Model 701) - Single phase inverter with split phase output"""
    
    # Model header
    model_id: int = 701
    model_length: int = 50
    
    # AC measurements
    ac_current: float = 0.0  # A - AC Total Current value
    ac_current_a: float = 0.0  # AphA - AC Phase A Current value
    ac_current_b: float = 0.0  # AphB - AC Phase B Current value
    ac_voltage_ab: float = 0.0  # PPVphAB - AC Voltage Phase AB value
    ac_power: float = 0.0  # W - AC Power value
    ac_frequency: float = 0.0  # Hz - AC Frequency value
    ac_energy: float = 0.0  # WH - AC Lifetime Energy production
    
    # DC measurements
    dc_current: float = 0.0  # DCA - DC Current value
    dc_voltage: float = 0.0  # DCV - DC Voltage value
    dc_power: float = 0.0  # DCW - DC Power value
    
    # Temperature
    cabinet_temperature: float = 0.0  # TmpCab - Cabinet Temperature
    
    # Status
    operating_state: int = 0  # St - Operating State
    vendor_operating_state: int = 0  # StVnd - Vendor Operating State
    
    # Scale factors (SF)
    current_sf: int = -2  # A_SF
    voltage_sf: int = -1  # V_SF
    power_sf: int = 0  # W_SF
    energy_sf: int = 0  # WH_SF
    frequency_sf: int = -2  # Hz_SF
    temperature_sf: int = 0  # Tmp_SF


@dataclass
class SunSpecBatteryModel:
    """SunSpec Battery Model (Model 713) - Battery bank model"""
    
    # Model header
    model_id: int = 713
    model_length: int = 30
    
    # Battery measurements
    battery_voltage: float = 0.0  # V - Battery voltage
    battery_current: float = 0.0  # A - Battery current
    battery_power: float = 0.0  # W - Battery power
    battery_soc: float = 0.0  # SoC - State of charge
    battery_temperature: float = 0.0  # Tmp - Battery temperature
    
    # Battery configuration
    battery_capacity: float = 0.0  # AHRtg - Amp-hour rating
    battery_energy_capacity: float = 0.0  # WHRtg - Watt-hour rating
    
    # Battery status
    battery_status: int = 0  # St - Battery status
    
    # Scale factors
    voltage_sf: int = -1  # V_SF
    current_sf: int = -2  # A_SF
    power_sf: int = 0  # W_SF
    energy_sf: int = 0  # WH_SF
    soc_sf: int = 0  # SoC_SF
    temperature_sf: int = 0  # Tmp_SF


class SunSpecRegisterMap:
    """SunSpec register mapping for Modbus TCP server"""
    
    # Base addresses for each model
    COMMON_MODEL_BASE = 40000
    INVERTER_MODEL_BASE = 40100
    BATTERY_MODEL_BASE = 40200
    
    # Common Model (1) registers
    SUNSPEC_ID = COMMON_MODEL_BASE  # 40000-40001 (2 registers)
    COMMON_MODEL_ID = COMMON_MODEL_BASE + 2  # 40002
    COMMON_MODEL_LENGTH = COMMON_MODEL_BASE + 3  # 40003
    MANUFACTURER = COMMON_MODEL_BASE + 4  # 40004-40019 (16 registers)
    MODEL = COMMON_MODEL_BASE + 20  # 40020-40035 (16 registers)
    OPTIONS = COMMON_MODEL_BASE + 36  # 40036-40043 (8 registers)
    VERSION = COMMON_MODEL_BASE + 44  # 40044-40051 (8 registers)
    SERIAL_NUMBER = COMMON_MODEL_BASE + 52  # 40052-40067 (16 registers)
    DEVICE_ADDRESS = COMMON_MODEL_BASE + 68  # 40068
    
    # Inverter Model (701) registers
    INVERTER_MODEL_ID = INVERTER_MODEL_BASE  # 40100
    INVERTER_MODEL_LENGTH = INVERTER_MODEL_BASE + 1  # 40101
    AC_CURRENT = INVERTER_MODEL_BASE + 2  # 40102
    AC_CURRENT_A = INVERTER_MODEL_BASE + 3  # 40103
    AC_CURRENT_B = INVERTER_MODEL_BASE + 4  # 40104
    AC_VOLTAGE_AB = INVERTER_MODEL_BASE + 5  # 40105
    AC_POWER = INVERTER_MODEL_BASE + 6  # 40106
    AC_FREQUENCY = INVERTER_MODEL_BASE + 7  # 40107
    AC_ENERGY = INVERTER_MODEL_BASE + 8  # 40108-40109 (2 registers)
    DC_CURRENT = INVERTER_MODEL_BASE + 10  # 40110
    DC_VOLTAGE = INVERTER_MODEL_BASE + 11  # 40111
    DC_POWER = INVERTER_MODEL_BASE + 12  # 40112
    CABINET_TEMP = INVERTER_MODEL_BASE + 13  # 40113
    OPERATING_STATE = INVERTER_MODEL_BASE + 14  # 40114
    VENDOR_STATE = INVERTER_MODEL_BASE + 15  # 40115
    
    # Battery Model (713) registers
    BATTERY_MODEL_ID = BATTERY_MODEL_BASE  # 40200
    BATTERY_MODEL_LENGTH = BATTERY_MODEL_BASE + 1  # 40201
    BATTERY_VOLTAGE = BATTERY_MODEL_BASE + 2  # 40202
    BATTERY_CURRENT = BATTERY_MODEL_BASE + 3  # 40203
    BATTERY_POWER = BATTERY_MODEL_BASE + 4  # 40204
    BATTERY_SOC = BATTERY_MODEL_BASE + 5  # 40205
    BATTERY_TEMP = BATTERY_MODEL_BASE + 6  # 40206
    BATTERY_CAPACITY = BATTERY_MODEL_BASE + 7  # 40207
    BATTERY_STATUS = BATTERY_MODEL_BASE + 8  # 40208


class SunSpecMapper:
    """Maps Sol-Ark data to SunSpec models"""
    
    def __init__(self, device_info: Dict[str, str]):
        """
        Initialize SunSpec mapper
        
        Args:
            device_info: Device information dictionary
        """
        self.logger = logging.getLogger(__name__)
        self.device_info = device_info
        
        # Initialize models
        self.common_model = SunSpecCommonModel(
            manufacturer=device_info.get("manufacturer", "Energy IoT Open Source"),
            model=device_info.get("model", "EMS-Dev Python"),
            options=device_info.get("options", "Sol-Ark Gateway"),
            version=device_info.get("version", "1.0.0"),
            serial_number=device_info.get("serial_number", "EMS-PY-001")
        )
        
        self.inverter_model = SunSpecInverterModel()
        self.battery_model = SunSpecBatteryModel()
        
        # Register map for Modbus server
        self.registers: Dict[int, int] = {}
        self._initialize_registers()
    
    def _initialize_registers(self):
        """Initialize the Modbus register map"""
        # Common Model registers
        self._set_register_32bit(SunSpecRegisterMap.SUNSPEC_ID, self.common_model.sunspec_id)
        self._set_register(SunSpecRegisterMap.COMMON_MODEL_ID, self.common_model.model_id)
        self._set_register(SunSpecRegisterMap.COMMON_MODEL_LENGTH, self.common_model.model_length)
        self._set_string_registers(SunSpecRegisterMap.MANUFACTURER, self.common_model.manufacturer, 16)
        self._set_string_registers(SunSpecRegisterMap.MODEL, self.common_model.model, 16)
        self._set_string_registers(SunSpecRegisterMap.OPTIONS, self.common_model.options, 8)
        self._set_string_registers(SunSpecRegisterMap.VERSION, self.common_model.version, 8)
        self._set_string_registers(SunSpecRegisterMap.SERIAL_NUMBER, self.common_model.serial_number, 16)
        self._set_register(SunSpecRegisterMap.DEVICE_ADDRESS, self.common_model.device_address)
        
        # Inverter Model registers
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_ID, self.inverter_model.model_id)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_LENGTH, self.inverter_model.model_length)
        
        # Battery Model registers
        self._set_register(SunSpecRegisterMap.BATTERY_MODEL_ID, self.battery_model.model_id)
        self._set_register(SunSpecRegisterMap.BATTERY_MODEL_LENGTH, self.battery_model.model_length)
    
    def _set_register(self, address: int, value: int):
        """Set a single register value"""
        self.registers[address] = value & 0xFFFF
    
    def _set_register_32bit(self, address: int, value: int):
        """Set a 32-bit value across two registers"""
        self.registers[address] = (value >> 16) & 0xFFFF
        self.registers[address + 1] = value & 0xFFFF
    
    def _set_string_registers(self, start_address: int, text: str, num_registers: int):
        """Set string value across multiple registers"""
        # Pad or truncate string to fit in registers
        text = text.ljust(num_registers * 2)[:num_registers * 2]
        
        for i in range(num_registers):
            char1 = ord(text[i * 2]) if i * 2 < len(text) else 0
            char2 = ord(text[i * 2 + 1]) if i * 2 + 1 < len(text) else 0
            value = (char1 << 8) | char2
            self.registers[start_address + i] = value
    
    def _scale_value(self, value: float, scale_factor: int) -> int:
        """Apply SunSpec scaling factor to a value"""
        if scale_factor >= 0:
            return int(value * (10 ** scale_factor))
        else:
            return int(value / (10 ** abs(scale_factor)))
    
    def update_from_solark(self, solark_data: SolArkData):
        """Update SunSpec models with Sol-Ark data"""
        try:
            # Update inverter model
            self.inverter_model.ac_current = abs(solark_data.inverter_current_l1 + solark_data.inverter_current_l2)
            self.inverter_model.ac_current_a = solark_data.inverter_current_l1
            self.inverter_model.ac_current_b = solark_data.inverter_current_l2
            self.inverter_model.ac_voltage_ab = solark_data.inverter_voltage
            self.inverter_model.ac_power = solark_data.inverter_output_power
            self.inverter_model.ac_frequency = solark_data.inverter_frequency
            self.inverter_model.ac_energy = solark_data.pv_energy * 1000  # Convert kWh to Wh
            
            # DC measurements (battery side)
            self.inverter_model.dc_current = abs(solark_data.battery_current)
            self.inverter_model.dc_voltage = solark_data.battery_voltage
            self.inverter_model.dc_power = abs(solark_data.battery_power)
            
            # Temperature
            self.inverter_model.cabinet_temperature = solark_data.battery_temperature
            
            # Operating state mapping
            self.inverter_model.operating_state = self._map_inverter_state(solark_data.inverter_status)
            self.inverter_model.vendor_operating_state = self._map_vendor_state(solark_data)
            
            # Update battery model
            self.battery_model.battery_voltage = solark_data.battery_voltage
            self.battery_model.battery_current = solark_data.battery_current
            self.battery_model.battery_power = solark_data.battery_power
            self.battery_model.battery_soc = solark_data.battery_soc
            self.battery_model.battery_temperature = solark_data.battery_temperature
            self.battery_model.battery_capacity = solark_data.battery_capacity
            self.battery_model.battery_energy_capacity = solark_data.battery_capacity * solark_data.battery_voltage
            self.battery_model.battery_status = self._map_battery_state(solark_data)
            
            # Update Modbus registers
            self._update_inverter_registers()
            self._update_battery_registers()
            
            self.logger.debug("Updated SunSpec models with Sol-Ark data")
            
        except Exception as e:
            self.logger.error(f"Error updating SunSpec models: {e}")
    
    def _map_inverter_state(self, inverter_status: int) -> int:
        """Map Sol-Ark inverter status to SunSpec operating state"""
        # Sol-Ark: 1=Self-test, 2=Normal, 3=Alarm, 4=Fault
        # SunSpec: 1=Off, 2=Sleeping, 4=Starting, 8=MPPT, 16=Throttled, 32=Shutting Down, 64=Fault, 128=Standby
        
        if inverter_status == 1:  # Self-test
            return 4  # Starting
        elif inverter_status == 2:  # Normal
            return 8  # MPPT
        elif inverter_status == 3:  # Alarm
            return 16  # Throttled
        elif inverter_status == 4:  # Fault
            return 64  # Fault
        else:
            return 1  # Off
    
    def _map_vendor_state(self, solark_data: SolArkData) -> int:
        """Map Sol-Ark data to vendor-specific state bits"""
        state = 0
        
        if solark_data.grid_relay_status > 0:
            state |= 0x0001  # Grid connected
        
        if solark_data.generator_relay_status > 0:
            state |= 0x0002  # Generator connected
        
        if solark_data.battery_power < 0:
            state |= 0x0004  # Battery charging
        
        if solark_data.battery_power > 0:
            state |= 0x0008  # Battery discharging
        
        if solark_data.grid_power < 0:
            state |= 0x0010  # Selling to grid
        
        if solark_data.grid_power > 0:
            state |= 0x0020  # Buying from grid
        
        return state
    
    def _map_battery_state(self, solark_data: SolArkData) -> int:
        """Map Sol-Ark battery data to battery status"""
        state = 0
        
        if solark_data.battery_power < 0:
            state |= 0x0001  # Charging
        elif solark_data.battery_power > 0:
            state |= 0x0002  # Discharging
        else:
            state |= 0x0004  # Idle
        
        if solark_data.bms_warning > 0:
            state |= 0x0008  # Warning
        
        if solark_data.bms_fault > 0:
            state |= 0x0010  # Fault
        
        return state
    
    def _update_inverter_registers(self):
        """Update inverter model registers"""
        # AC measurements
        self._set_register(SunSpecRegisterMap.AC_CURRENT, 
                          self._scale_value(self.inverter_model.ac_current, self.inverter_model.current_sf))
        self._set_register(SunSpecRegisterMap.AC_CURRENT_A, 
                          self._scale_value(self.inverter_model.ac_current_a, self.inverter_model.current_sf))
        self._set_register(SunSpecRegisterMap.AC_CURRENT_B, 
                          self._scale_value(self.inverter_model.ac_current_b, self.inverter_model.current_sf))
        self._set_register(SunSpecRegisterMap.AC_VOLTAGE_AB, 
                          self._scale_value(self.inverter_model.ac_voltage_ab, self.inverter_model.voltage_sf))
        self._set_register(SunSpecRegisterMap.AC_POWER, 
                          self._scale_value(self.inverter_model.ac_power, self.inverter_model.power_sf))
        self._set_register(SunSpecRegisterMap.AC_FREQUENCY, 
                          self._scale_value(self.inverter_model.ac_frequency, self.inverter_model.frequency_sf))
        self._set_register_32bit(SunSpecRegisterMap.AC_ENERGY, 
                                int(self.inverter_model.ac_energy))
        
        # DC measurements
        self._set_register(SunSpecRegisterMap.DC_CURRENT, 
                          self._scale_value(self.inverter_model.dc_current, self.inverter_model.current_sf))
        self._set_register(SunSpecRegisterMap.DC_VOLTAGE, 
                          self._scale_value(self.inverter_model.dc_voltage, self.inverter_model.voltage_sf))
        self._set_register(SunSpecRegisterMap.DC_POWER, 
                          self._scale_value(self.inverter_model.dc_power, self.inverter_model.power_sf))
        
        # Temperature and status
        self._set_register(SunSpecRegisterMap.CABINET_TEMP, 
                          self._scale_value(self.inverter_model.cabinet_temperature, self.inverter_model.temperature_sf))
        self._set_register(SunSpecRegisterMap.OPERATING_STATE, self.inverter_model.operating_state)
        self._set_register(SunSpecRegisterMap.VENDOR_STATE, self.inverter_model.vendor_operating_state)
    
    def _update_battery_registers(self):
        """Update battery model registers"""
        self._set_register(SunSpecRegisterMap.BATTERY_VOLTAGE, 
                          self._scale_value(self.battery_model.battery_voltage, self.battery_model.voltage_sf))
        self._set_register(SunSpecRegisterMap.BATTERY_CURRENT, 
                          self._scale_value(self.battery_model.battery_current, self.battery_model.current_sf))
        self._set_register(SunSpecRegisterMap.BATTERY_POWER, 
                          self._scale_value(self.battery_model.battery_power, self.battery_model.power_sf))
        self._set_register(SunSpecRegisterMap.BATTERY_SOC, 
                          self._scale_value(self.battery_model.battery_soc, self.battery_model.soc_sf))
        self._set_register(SunSpecRegisterMap.BATTERY_TEMP, 
                          self._scale_value(self.battery_model.battery_temperature, self.battery_model.temperature_sf))
        self._set_register(SunSpecRegisterMap.BATTERY_CAPACITY, 
                          self._scale_value(self.battery_model.battery_capacity, 0))
        self._set_register(SunSpecRegisterMap.BATTERY_STATUS, self.battery_model.battery_status)
    
    def get_register_value(self, address: int) -> Optional[int]:
        """Get register value by address"""
        return self.registers.get(address)
    
    def get_all_registers(self) -> Dict[int, int]:
        """Get all register values"""
        return self.registers.copy()