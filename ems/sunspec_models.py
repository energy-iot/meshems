"""
SunSpec Models Implementation

This module implements SunSpec-compliant data models for exposing Sol-Ark inverter data
in a standardized format over Modbus TCP.
"""

import logging
import time
from dataclasses import dataclass, field

from .solark_client import SolArkData


@dataclass
class SunSpecCommonModel:
    """SunSpec Common Model (Model 1) - Device identification"""
    
    # Fixed header
    sunspec_id = 0x53756E53  # 'SunS' in ASCII
    model_id = 1
    model_length = 66  # Match C implementation
    
    # Device information
    manufacturer = "Energy IoT Open Source"
    model = "EMS-Dev Python"
    options = "Sol-Ark Gateway"
    version = "1.0.0"
    serial_number = "EMS-PY-001"
    device_address = 1


@dataclass
class SunSpecInverterModel:
    """SunSpec Inverter Model (Model 701) - Single phase inverter with split phase output"""
    
    # Model header
    model_id = 701
    model_length = 153  # Match C implementation
    
    # AC measurements
    ac_current = 0.0  # A - AC Total Current value
    ac_current_a = 0.0  # AphA - AC Phase A Current value
    ac_current_b = 0.0  # AphB - AC Phase B Current value
    ac_voltage_ab = 0.0  # PPVphAB - AC Voltage Phase AB value
    ac_power = 0.0  # W - AC Power value
    ac_frequency = 0.0  # Hz - AC Frequency value
    ac_energy = 0.0  # WH - AC Lifetime Energy production
    
    # DC measurements
    dc_current = 0.0  # DCA - DC Current value
    dc_voltage = 0.0  # DCV - DC Voltage value
    dc_power = 0.0  # DCW - DC Power value
    
    # Temperature
    cabinet_temperature = 0.0  # TmpCab - Cabinet Temperature
    
    # Status
    operating_state = 0  # St - Operating State
    vendor_operating_state = 0  # StVnd - Vendor Operating State
    
    # Scale factors (SF)
    current_sf = -2  # A_SF
    voltage_sf = -1  # V_SF
    power_sf = 0  # W_SF
    energy_sf = 0  # WH_SF
    frequency_sf = -2  # Hz_SF
    temperature_sf = 0  # Tmp_SF


@dataclass
class SunSpecBatteryModel:
    """SunSpec Battery Model (Model 713) - Battery bank model"""
    
    # Model header
    model_id = 713
    model_length = 7  # Match C implementation (DER Storage Capacity Model)
    
    # Battery measurements
    battery_voltage = 0.0  # V - Battery voltage
    battery_current = 0.0  # A - Battery current
    battery_power = 0.0  # W - Battery power
    battery_soc = 0.0  # SoC - State of charge
    battery_temperature = 0.0  # Tmp - Battery temperature
    
    # Battery configuration
    battery_capacity = 0.0  # AHRtg - Amp-hour rating
    battery_energy_capacity = 0.0  # WHRtg - Watt-hour rating
    
    # Battery status
    battery_status = 0  # St - Battery status
    
    # Scale factors
    voltage_sf = -1  # V_SF
    current_sf = -2  # A_SF
    power_sf = 0  # W_SF
    energy_sf = 0  # WH_SF
    soc_sf = 0  # SoC_SF
    temperature_sf = 0  # Tmp_SF


class SunSpecRegisterMap:
    """SunSpec register mapping for Modbus TCP server"""
    
    # Base addresses for each model (matching C implementation exactly)
    SUNSPEC_BASE_ADDR = 40000
    COMMON_MODEL_BASE = SUNSPEC_BASE_ADDR + 2  # 40002 (after SunS header)
    INVERTER_MODEL_BASE = COMMON_MODEL_BASE + 66 + 2  # 40070 (after Common Model + header)
    STORAGE_MODEL_BASE = INVERTER_MODEL_BASE + 153 + 2  # 40225 (after Inverter Model + header)
    END_MODEL_BASE = STORAGE_MODEL_BASE + 7 + 2  # 40234 (after Storage Model + header)
    
    # SunSpec Header
    SUNSPEC_ID = SUNSPEC_BASE_ADDR  # 40000-40001 (2 registers)
    
    # Common Model (1) registers 
    COMMON_MODEL_ID = COMMON_MODEL_BASE + 0  # 40002
    COMMON_MODEL_LENGTH = COMMON_MODEL_BASE + 1  # 40003
    MANUFACTURER = COMMON_MODEL_BASE + 2  # 40004-40019 (16 registers)
    MODEL = COMMON_MODEL_BASE + 18  # 40020-40035 (16 registers)
    OPTIONS = COMMON_MODEL_BASE + 34  # 40036-40043 (8 registers)
    VERSION = COMMON_MODEL_BASE + 42  # 40044-40051 (8 registers)
    SERIAL_NUMBER = COMMON_MODEL_BASE + 50  # 40052-40067 (16 registers)
    DEVICE_ADDRESS = COMMON_MODEL_BASE + 66  # 40068
    
    # Inverter Model (701) registers 
    INVERTER_MODEL_ID = INVERTER_MODEL_BASE  # 40068
    INVERTER_MODEL_LENGTH = INVERTER_MODEL_BASE + 1  # 40069
    INV_AC_TYPE = INVERTER_MODEL_BASE + 2  # 40070
    INV_OPERATING_STATE = INVERTER_MODEL_BASE + 3  # 40071
    INV_STATUS = INVERTER_MODEL_BASE + 4  # 40072
    INV_GRID_CONNECTION = INVERTER_MODEL_BASE + 5  # 40073
    INV_ALARM = INVERTER_MODEL_BASE + 6  # 40074-40075 (2 registers)
    INV_DER_MODE = INVERTER_MODEL_BASE + 8  # 40076-40077 (2 registers)
    INV_AC_POWER = INVERTER_MODEL_BASE + 10  # 40078
    INV_AC_VA = INVERTER_MODEL_BASE + 11  # 40079
    INV_AC_VAR = INVERTER_MODEL_BASE + 12  # 40080
    INV_AC_PF = INVERTER_MODEL_BASE + 13  # 40081
    INV_AC_CURRENT = INVERTER_MODEL_BASE + 14  # 40082
    INV_AC_VOLTAGE_LL = INVERTER_MODEL_BASE + 15  # 40083
    INV_AC_VOLTAGE_LN = INVERTER_MODEL_BASE + 16  # 40084
    INV_AC_FREQUENCY = INVERTER_MODEL_BASE + 17  # 40085-40086 (2 registers)
    
    # Storage Model (713) registers 
    STORAGE_MODEL_ID = STORAGE_MODEL_BASE + 0  # 40225
    STORAGE_MODEL_LENGTH = STORAGE_MODEL_BASE + 1  # 40226
    STORAGE_ENERGY_RATING = STORAGE_MODEL_BASE + 2  # 40227
    STORAGE_ENERGY_AVAILABLE = STORAGE_MODEL_BASE + 3  # 40228
    STORAGE_SOC = STORAGE_MODEL_BASE + 4  # 40229
    STORAGE_SOH = STORAGE_MODEL_BASE + 5  # 40230
    STORAGE_STATUS = STORAGE_MODEL_BASE + 6  # 40231
    STORAGE_SF_ENERGY = STORAGE_MODEL_BASE + 7  # 40232
    STORAGE_SF_PERCENT = STORAGE_MODEL_BASE + 8  # 40233
    
    # End-of-map marker
    END_MODEL_ID = END_MODEL_BASE + 0  # 40234 - Model ID 65535 (0xFFFF)
    END_MODEL_LENGTH = END_MODEL_BASE + 1  # 40235 - Length 0


class SunSpecMapper:
    """Maps Sol-Ark data to SunSpec models"""
    
    def __init__(self, device_info):
        """
        Initialize SunSpec mapper
        
        Args:
            device_info: Device information dictionary
        """
        self.logger = logging.getLogger(__name__)
        self.device_info = device_info
        
        # Initialize models
        self.common_model = SunSpecCommonModel()
        
        # Update with device info
        if "manufacturer" in device_info:
            self.common_model.manufacturer = device_info["manufacturer"]
        if "model" in device_info:
            self.common_model.model = device_info["model"]
        if "options" in device_info:
            self.common_model.options = device_info["options"]
        if "version" in device_info:
            self.common_model.version = device_info["version"]
        if "serial_number" in device_info:
            self.common_model.serial_number = device_info["serial_number"]
        
        self.inverter_model = SunSpecInverterModel()
        self.battery_model = SunSpecBatteryModel()
        
        # Register map for Modbus server
        self.registers = {}
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
        
        # Inverter Model header
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_ID, self.inverter_model.model_id)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_LENGTH, self.inverter_model.model_length)
        
        # Initialize all inverter model values to "not implemented" (0xFFFF)
        for i in range(2, self.inverter_model.model_length + 2):  # +2 for header
            self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + i, 0xFFFF)
        
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 35, 0x7FFF) #set TmpAmb to null
        
        # Set scale factors 
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 113, -2)  # Current scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 114, -1)  # Voltage scale factor: -1 (0.1)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 115, -2)  # Frequency scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 116, 0)   # Power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 117, -2)  # Power factor scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 118, 0)   # Apparent power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 119, 0)   # Reactive power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 120, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 121, -3)  # Reactive energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 122, -1)  # Temperature scale factor: -1 (0.1)
        
        # Initialize string fields to empty/null instead of 0xFFFF
        # INV_ALARM_INFO (32 registers starting at offset 123)
        self._set_string_registers(SunSpecRegisterMap.INVERTER_MODEL_BASE + 123, "", 32)
        
        # Storage Model header
        self._set_register(SunSpecRegisterMap.STORAGE_MODEL_ID, self.battery_model.model_id)
        self._set_register(SunSpecRegisterMap.STORAGE_MODEL_LENGTH, self.battery_model.model_length)
        
        # Initialize all storage model values to "not implemented" (0xFFFF)
        for i in range(2, self.battery_model.model_length + 2):  # +2 for header
            self._set_register(SunSpecRegisterMap.STORAGE_MODEL_BASE + i, 0xFFFF)
        
        # Set scale factors for storage model 
        self._set_register(SunSpecRegisterMap.STORAGE_SF_ENERGY, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.STORAGE_SF_PERCENT, -1)  # Percentage scale factor: -1 (0.1)
        
        # End-of-map marker (Model ID 65535, Length 0)
        self._set_register(SunSpecRegisterMap.END_MODEL_ID, 65535)  # 0xFFFF
        self._set_register(SunSpecRegisterMap.END_MODEL_LENGTH, 0)
    
    def _set_register(self, address, value):
        """Set a single register value"""
        self.registers[address] = value & 0xFFFF
    
    def _set_register_32bit(self, address, value):
        """Set a 32-bit value across two registers"""
        self.registers[address] = (value >> 16) & 0xFFFF
        self.registers[address + 1] = value & 0xFFFF
    
    def _set_string_registers(self, start_address, text, num_registers):
        """Set string value across multiple registers"""
        # Pad or truncate string to fit in registers
        text = text.ljust(num_registers * 2)[:num_registers * 2]
        
        for i in range(num_registers):
            char1 = ord(text[i * 2]) if i * 2 < len(text) else 0
            char2 = ord(text[i * 2 + 1]) if i * 2 + 1 < len(text) else 0
            value = (char1 << 8) | char2
            self.registers[start_address + i] = value
    
    def _scale_value(self, value, scale_factor):
        """Apply SunSpec scaling factor to a value"""
        if scale_factor >= 0:
            return int(value * (10 ** scale_factor))
        else:
            return int(value / (10 ** abs(scale_factor)))
    
    def update_from_solark(self, solark_data):
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
            self.inverter_model.cabinet_temperature = 0x7FFF
            
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
            self.battery_model.battery_status = self._map_storage_status(solark_data)
            
            # Update Modbus registers
            self.update_inverter_from_solark(solark_data)
            self._update_battery_registers()
            
            self.logger.debug("Updated SunSpec models with Sol-Ark data")
            
        except Exception as e:
            self.logger.error(f"Error updating SunSpec models: {e}")
    
    def _map_inverter_state(self, inverter_status):
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
    
    def _map_vendor_state(self, solark_data):
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
    
    def _map_storage_status(self, solark_data):
        """Map Sol-Ark BMS data to SunSpec storage status """
        # SunSpec Storage Status enumeration:
        # 0 = OK
        # 1 = Warning
        # 2 = Error/Fault
        
        if solark_data.bms_fault > 0:
            return 2  # Error
        elif solark_data.bms_warning > 0:
            return 1  # Warning
        else:
            return 0  # OK
    
    def _update_inverter_registers(self):
        """Update inverter model registers"""
        # This method should be called with actual Sol-Ark data
        # For now, we'll use default values - this will be updated when called with real data
        pass
    
    def update_inverter_from_solark(self, solark_data):
        """Update inverter registers with actual Sol-Ark data """
        # Set AC wiring type based on Sol-Ark register 286 (Grid Type) - matching C implementation
        # Sol-Ark Grid Type: 0x00=Single-phase, 0x01=Split-phase, 0x02=Three-phase Wye
        # SunSpec INV_AC_TYPE (enum ACType): 0=Unknown, 1=Split Phase, 2=Single Phase, 3=Three Phase Wye
        sunspec_ac_type = 0  # Default to Unknown
        if solark_data.grid_type == 0x00:  # Single-phase
            sunspec_ac_type = 2  # Single Phase
        elif solark_data.grid_type == 0x01:  # Split-phase
            sunspec_ac_type = 1  # Split Phase
        elif solark_data.grid_type == 0x02:  # Three-phase Wye
            sunspec_ac_type = 3  # Three Phase Wye
        
        self._set_register(SunSpecRegisterMap.INV_AC_TYPE, sunspec_ac_type)
        # Set operating state based on Sol-Ark inverter status (register 59) - matching C implementation
        self._set_register(SunSpecRegisterMap.INV_OPERATING_STATE, 1 if solark_data.inverter_status == 2 else 0)  # 0=Off, 1=On (only "Normal" is considered "On")
        
        # Set inverter state based on Sol-Ark status mapping - matching C implementation
        # Sol-Ark States: 1=Self-test, 2=Normal, 3=Alarm, 4=Fault
        # SunSpec States: 0=OFF, 1=SLEEPING, 2=STARTING, 3=RUNNING, 4=THROTTLED, 5=SHUTTING_DOWN, 6=FAULT, 7=STANDBY
        inv_state = 0  # Default to OFF
        if solark_data.inverter_status == 1:  # Self-test
            inv_state = 2  # STARTING
        elif solark_data.inverter_status == 2:  # Normal
            if solark_data.inverter_output_power > 100:
                inv_state = 3  # RUNNING
            else:
                inv_state = 7  # STANDBY
        elif solark_data.inverter_status == 3:  # Alarm
            inv_state = 4  # THROTTLED (closest match for alarm state)
        elif solark_data.inverter_status == 4:  # Fault
            inv_state = 6  # FAULT
        else:
            inv_state = 0  # OFF
        
        self._set_register(SunSpecRegisterMap.INV_STATUS, inv_state)
        
        # Grid connection state 
        # Sol-Ark register 194: 1 = Open (Disconnected), 2 = Closed (Connected)
        grid_connected = 1 if solark_data.grid_relay_status == 2 else 0
        self._set_register(SunSpecRegisterMap.INV_GRID_CONNECTION, grid_connected)
        
        # Set DER operational characteristics (matching C implementation exactly)
        # INV_DER_MODE (DERCtl bitmask): Bit 0 = GridFollowing (0x0000), Bit 1 = GridForming (0x0001)
        der_mode = 0
        if solark_data.grid_relay_status == 2:  # Sol-Ark relay closed (Connected to grid)
            der_mode |= 0x0000  # Set Grid Following
        else:  # Sol-Ark relay open (Disconnected from grid)
            der_mode |= 0x0001  # Set Grid Forming
        
        # Store 32-bit DER mode across two registers 
        self._set_register(SunSpecRegisterMap.INV_DER_MODE, (der_mode >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.INV_DER_MODE + 1, der_mode & 0xFFFF)
        
        # Alarm bitfield (not implemented in this example)
        self._set_register(SunSpecRegisterMap.INV_ALARM, 0)
        self._set_register(SunSpecRegisterMap.INV_ALARM + 1, 0)
        
        # Power measurements
        self._set_register(SunSpecRegisterMap.INV_AC_POWER, int(solark_data.inverter_output_power))
        self._set_register(SunSpecRegisterMap.INV_AC_VA, int(solark_data.inverter_output_power))  # Approximation
        self._set_register(SunSpecRegisterMap.INV_AC_VAR, 0)  # Not available
        self._set_register(SunSpecRegisterMap.INV_AC_PF, 100)  # Power factor 1.0 (scaled by 100)
        
        # Current and voltage measurements
        ac_current_total = (solark_data.inverter_current_l1 + solark_data.inverter_current_l2) / 2.0
        self._set_register(SunSpecRegisterMap.INV_AC_CURRENT, int(ac_current_total * 100))  # Scale by 100 for 0.01 scale factor
        self._set_register(SunSpecRegisterMap.INV_AC_VOLTAGE_LL, int(solark_data.inverter_voltage * 10))  # Scale by 10 for 0.1 scale factor
        self._set_register(SunSpecRegisterMap.INV_AC_VOLTAGE_LN, int(solark_data.inverter_voltage_ln * 10))  # VL1: Line1-to-Neutral voltage from register 154
        
        # Frequency (32-bit value, matching C implementation)
        frequency_scaled = int(solark_data.inverter_frequency * 100)  # Scale by 100 for 0.01 scale factor
        self._set_register(SunSpecRegisterMap.INV_AC_FREQUENCY, (frequency_scaled >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.INV_AC_FREQUENCY + 1, frequency_scaled & 0xFFFF)
        
        # Energy measurements (4 registers each for uint64, matching C implementation)
        energy_wh = int(solark_data.load_energy * 1000)  # Convert kWh to Wh
        # For simplicity, we'll use the lower 32 bits (could be extended to full 64-bit)
        energy_low32 = energy_wh & 0xFFFFFFFF
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 19, 0)  # High 32 bits (upper 16)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 20, 0)  # High 32 bits (lower 16)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 21, (energy_low32 >> 16) & 0xFFFF)  # Low 32 bits (upper 16)
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 22, energy_low32 & 0xFFFF)  # Low 32 bits (lower 16)
        
        # Temperature measurements 
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 36, int(solark_data.battery_temperature * 10))  # Cabinet temp
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 38, int(solark_data.dcdc_xfrmr_temp * 10))  # Transformer temp
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 39, int(solark_data.igbt_temp * 10))  # IGBT temp
        
        # Phase L1 measurements 
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 41, int(solark_data.inverter_power_l1))  # WL1: AC Power L1 from register 173
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 45, int(solark_data.inverter_current_l1 * 100))  # Current L1
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 46, int(solark_data.inverter_voltage_ln * 10))  # VL1: Line1-to-Neutral voltage
        
        # Phase L2 measurements
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 64, int(solark_data.inverter_power_l2))  # WL2: AC Power L2 from register 174
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 68, int(solark_data.inverter_current_l2 * 100))  # Current L2
        self._set_register(SunSpecRegisterMap.INVERTER_MODEL_BASE + 69, int(solark_data.inverter_voltage_l2n * 10))  # VL2: Line2-to-Neutral voltage from register 155
        
        # Vendor-specific status information in the alarm info field 
        alarm_info = f"Grid:{'Connected' if solark_data.grid_relay_status == 2 else 'Disconnected'} Batt:{'Charging' if solark_data.battery_power < 0 else ('Discharging' if solark_data.battery_power > 0 else 'Idle')}"
        self._set_string_registers(SunSpecRegisterMap.INVERTER_MODEL_BASE + 123, alarm_info, 32)
    
    def _update_battery_registers(self):
        """Update storage model registers """
        # Calculate battery energy rating from capacity (Ah) and nominal voltage
        battery_capacity_ah = self.battery_model.battery_capacity
        nominal_voltage = 51.2  # Typical 48V LFP system, could be made configurable
        
        energy_rating = int(battery_capacity_ah * nominal_voltage)  # Wh = Ah * V
        self._set_register(SunSpecRegisterMap.STORAGE_ENERGY_RATING, energy_rating)
        
        # Energy available (WHAvail = WHRtg * SoC * SoH)
        # Using actual SOC from Sol-Ark and assuming 100% SoH if not available from BMS
        soc = self.battery_model.battery_soc / 100.0
        soh = 1.0  # Default to 100% if BMS data not available
        
        energy_available = int(energy_rating * soc * soh)
        self._set_register(SunSpecRegisterMap.STORAGE_ENERGY_AVAILABLE, energy_available)
        
        # State of charge (%) - scaled by 10 for 0.1 scale factor
        self._set_register(SunSpecRegisterMap.STORAGE_SOC, int(self.battery_model.battery_soc * 10))
        
        # State of health (%) - default to 100% since Sol-Ark doesn't provide this directly
        # In a real implementation, this could be calculated from battery degradation over time
        self._set_register(SunSpecRegisterMap.STORAGE_SOH, 1000)  # 100.0% (scaled by 10)
        
        # Storage status - use the corrected enumerated values (0=OK, 1=Warning, 2=Error)
        self._set_register(SunSpecRegisterMap.STORAGE_STATUS, self.battery_model.battery_status)
        
        # Scale factors 
        self._set_register(SunSpecRegisterMap.STORAGE_SF_ENERGY, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.STORAGE_SF_PERCENT, -1)  # Percentage scale factor: -1 (0.1)
    
    def get_register_value(self, address):
        """Get register value by address"""
        return self.registers.get(address)
    
    def get_all_registers(self):
        """Get all register values"""
        return self.registers.copy()