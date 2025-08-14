"""
SunSpec Models Implementation

This module implements SunSpec-compliant data models for exposing inverter data
in a standardized format over Modbus TCP.
"""

import logging
import time
from dataclasses import dataclass, field

from .base import InverterData


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
    options = "Generic Inverter Gateway"
    version = "1.0.0"
    serial_number = "EMS-PY-001"
    device_address = 1


@dataclass
class SunSpecInverterModel:
    """SunSpec Inverter Model (Model 701) - Base class for Grid and Load instances"""
    
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
class SunSpecGridModel(SunSpecInverterModel):
    """SunSpec Grid Model (Model 701) - Grid-side measurements"""
    
    # Grid-specific measurements
    grid_power: float = 0.0  # W - Total grid power
    grid_power_l1: float = 0.0  # WL1 - Grid power L1
    grid_power_l2: float = 0.0  # WL2 - Grid power L2
    grid_power_l3: float = 0.0  # WL3 - Grid power L3
    
    # Grid voltages
    grid_voltage_l1l2: float = 0.0  # LLV - Line-to-line voltage L1-L2
    grid_voltage_l2l3: float = 0.0  # LLV - Line-to-line voltage L2-L3
    grid_voltage_l3l1: float = 0.0  # LLV - Line-to-line voltage L3-L1
    grid_voltage_l1n: float = 0.0   # LNV - Line-to-neutral voltage L1-N
    grid_voltage_l2n: float = 0.0   # LNV - Line-to-neutral voltage L2-N
    grid_voltage_l3n: float = 0.0   # LNV - Line-to-neutral voltage L3-N
    
    # Grid currents
    grid_current_l1: float = 0.0  # AL1 - Grid current L1
    grid_current_l2: float = 0.0  # AL2 - Grid current L2
    grid_current_l3: float = 0.0  # AL3 - Grid current L3
    
    # Grid frequency and power factor
    grid_frequency: float = 0.0      # Hz - Grid frequency
    grid_power_factor: float = 0.0   # PF - Grid power factor
    
    # Grid status and connection
    grid_connection_status: int = 0  # Connection status
    grid_relay_status: int = 0       # Relay status
    
    def update_from_data(self, data):
        """Update grid model from inverter data"""
        if hasattr(data, 'grid_power'):
            self.grid_power = data.grid_power
        if hasattr(data, 'grid_power_l1'):
            self.grid_power_l1 = data.grid_power_l1
        if hasattr(data, 'grid_power_l2'):
            self.grid_power_l2 = data.grid_power_l2
        if hasattr(data, 'grid_power_l3'):
            self.grid_power_l3 = data.grid_power_l3
            
        if hasattr(data, 'grid_voltage_l1l2'):
            self.grid_voltage_l1l2 = data.grid_voltage_l1l2
        if hasattr(data, 'grid_voltage_l2l3'):
            self.grid_voltage_l2l3 = data.grid_voltage_l2l3
        if hasattr(data, 'grid_voltage_l3l1'):
            self.grid_voltage_l3l1 = data.grid_voltage_l3l1
        if hasattr(data, 'grid_voltage_l1n'):
            self.grid_voltage_l1n = data.grid_voltage_l1n
        if hasattr(data, 'grid_voltage_l2n'):
            self.grid_voltage_l2n = data.grid_voltage_l2n
        if hasattr(data, 'grid_voltage_l3n'):
            self.grid_voltage_l3n = data.grid_voltage_l3n
            
        if hasattr(data, 'grid_current_l1'):
            self.grid_current_l1 = data.grid_current_l1
        if hasattr(data, 'grid_current_l2'):
            self.grid_current_l2 = data.grid_current_l2
        if hasattr(data, 'grid_current_l3'):
            self.grid_current_l3 = data.grid_current_l3
            
        if hasattr(data, 'grid_frequency'):
            self.grid_frequency = data.grid_frequency
        if hasattr(data, 'grid_power_factor'):
            self.grid_power_factor = data.grid_power_factor
        if hasattr(data, 'grid_relay_status'):
            self.grid_relay_status = data.grid_relay_status


@dataclass
class SunSpecLoadModel(SunSpecInverterModel):
    """SunSpec Load Model (Model 701) - Load-side measurements"""
    
    # Load-specific measurements
    load_power_total: float = 0.0  # W - Total load power
    load_power_l1: float = 0.0     # WL1 - Load power L1
    load_power_l2: float = 0.0     # WL2 - Load power L2
    load_power_l3: float = 0.0     # WL3 - Load power L3
    
    # Load voltages
    load_voltage_l1l2: float = 0.0  # LLV - Line-to-line voltage L1-L2
    load_voltage_l2l3: float = 0.0  # LLV - Line-to-line voltage L2-L3
    load_voltage_l3l1: float = 0.0  # LLV - Line-to-line voltage L3-L1
    load_voltage_l1n: float = 0.0   # LNV - Line-to-neutral voltage L1-N
    load_voltage_l2n: float = 0.0   # LNV - Line-to-neutral voltage L2-N
    load_voltage_l3n: float = 0.0   # LNV - Line-to-neutral voltage L3-N
    
    # Load currents
    load_current_l1: float = 0.0  # AL1 - Load current L1
    load_current_l2: float = 0.0  # AL2 - Load current L2
    load_current_l3: float = 0.0  # AL3 - Load current L3
    
    # Load frequency and power factor
    load_frequency: float = 0.0      # Hz - Load frequency
    load_power_factor: float = 0.0   # PF - Load power factor
    
    # Load energy counters
    load_energy_total: float = 0.0   # WH - Total load energy
    
    def update_from_data(self, data):
        """Update load model from inverter data"""
        if hasattr(data, 'load_power_total'):
            self.load_power_total = data.load_power_total
        if hasattr(data, 'load_power_l1'):
            self.load_power_l1 = data.load_power_l1
        if hasattr(data, 'load_power_l2'):
            self.load_power_l2 = data.load_power_l2
        if hasattr(data, 'load_power_l3'):
            self.load_power_l3 = data.load_power_l3
            
        if hasattr(data, 'load_voltage_l1l2'):
            self.load_voltage_l1l2 = data.load_voltage_l1l2
        if hasattr(data, 'load_voltage_l2l3'):
            self.load_voltage_l2l3 = data.load_voltage_l2l3
        if hasattr(data, 'load_voltage_l3l1'):
            self.load_voltage_l3l1 = data.load_voltage_l3l1
        if hasattr(data, 'load_voltage_l1n'):
            self.load_voltage_l1n = data.load_voltage_l1n
        if hasattr(data, 'load_voltage_l2n'):
            self.load_voltage_l2n = data.load_voltage_l2n
        if hasattr(data, 'load_voltage_l3n'):
            self.load_voltage_l3n = data.load_voltage_l3n
            
        if hasattr(data, 'load_current_l1'):
            self.load_current_l1 = data.load_current_l1
        if hasattr(data, 'load_current_l2'):
            self.load_current_l2 = data.load_current_l2
        if hasattr(data, 'load_current_l3'):
            self.load_current_l3 = data.load_current_l3
            
        if hasattr(data, 'load_frequency'):
            self.load_frequency = data.load_frequency
        if hasattr(data, 'load_energy'):
            self.load_energy_total = data.load_energy


@dataclass
class SunSpecBatteryModel:
    """SunSpec Battery Model (Model 713) - Battery bank model"""
    
    # Model header
    model_id = 713
    model_length = 7  # Match C implementation (DER Storage Capacity Model)
    
    # Battery measurements
    battery_voltage: float = 0.0  # V - Battery voltage
    battery_current: float = 0.0  # A - Battery current
    battery_power: float = 0.0  # W - Battery power
    battery_soc: float = 0.0  # SoC - State of charge
    battery_temperature: float = 0.0  # Tmp - Battery temperature
    
    # Battery configuration
    battery_capacity: float = 0.0  # AHRtg - Amp-hour rating
    battery_energy_capacity: float = 0.0  # WHRtg - Watt-hour rating
    
    # Battery status and health
    battery_status: int = 0  # St - Battery status (0=OFF, 1=EMPTY, 2=DISCHARGING, 3=CHARGING, 4=FULL, 5=HOLDING, 6=TESTING)
    battery_health: float = 100.0  # SoH - State of health (%)
    
    # Battery energy counters
    battery_energy_charged: float = 0.0  # WhCha - Total energy charged
    battery_energy_discharged: float = 0.0  # WhDisCha - Total energy discharged
    
    # Scale factors
    voltage_sf: int = -1  # V_SF
    current_sf: int = -2  # A_SF
    power_sf: int = 0  # W_SF
    energy_sf: int = 0  # WH_SF
    soc_sf: int = 0  # SoC_SF
    temperature_sf: int = 0  # Tmp_SF
    
    def update_from_data(self, data):
        """Update battery model from inverter data"""
        if hasattr(data, 'battery_voltage'):
            self.battery_voltage = data.battery_voltage
        if hasattr(data, 'battery_current'):
            self.battery_current = data.battery_current
        if hasattr(data, 'battery_power'):
            self.battery_power = data.battery_power
        if hasattr(data, 'battery_soc'):
            self.battery_soc = data.battery_soc
        if hasattr(data, 'battery_temperature'):
            self.battery_temperature = data.battery_temperature
            
        if hasattr(data, 'battery_capacity'):
            self.battery_capacity = data.battery_capacity
        if hasattr(data, 'battery_energy_capacity'):
            self.battery_energy_capacity = data.battery_energy_capacity
            
        # Determine battery status based on power flow
        if hasattr(data, 'battery_power') and data.battery_power is not None:
            if data.battery_power > 50:
                self.battery_status = 3  # Charging
            elif data.battery_power < -50:
                self.battery_status = 2  # Discharging
            elif hasattr(data, 'battery_soc') and data.battery_soc is not None:
                if data.battery_soc >= 95:
                    self.battery_status = 4  # Full
                elif data.battery_soc <= 5:
                    self.battery_status = 1  # Empty
                else:
                    self.battery_status = 5  # Holding
            else:
                self.battery_status = 5  # Holding (default)
        
        # Update energy counters if available
        if hasattr(data, 'battery_energy_charged'):
            self.battery_energy_charged = data.battery_energy_charged
        if hasattr(data, 'battery_energy_discharged'):
            self.battery_energy_discharged = data.battery_energy_discharged
            
        # Update health if available
        if hasattr(data, 'battery_health'):
            self.battery_health = data.battery_health


@dataclass
class SunSpecDCPort:
    """SunSpec DC Port for Model 714"""
    
    # Port identification
    port_type: int = 0  # PrtTyp - 0=PV, 1=ESS, 2=EV, 3=INJ, 4=ABS, 5=BIDIR, 6=DC_DC
    port_id: int = 0  # ID - Port ID number
    port_id_string: str = ""  # IDStr - Port ID string (8 registers)
    
    # DC measurements
    dc_current: float = 0.0  # DCA - DC current for the port
    dc_voltage: float = 0.0  # DCV - DC voltage for the port
    dc_power: float = 0.0  # DCW - DC power for the port
    dc_energy_injected: int = 0  # DCWhInj - Total cumulative DC energy injected
    dc_energy_absorbed: int = 0  # DCWhAbs - Total cumulative DC energy absorbed
    
    # Status and temperature
    temperature: float = 0.0  # Tmp - DC port temperature
    dc_status: int = 0  # DCSta - 0=OFF, 1=ON, 2=WARNING, 3=ERROR
    dc_alarm: int = 0  # DCAlrm - DC port alarm bitfield (32-bit)


@dataclass
class SunSpec714Model:
    """SunSpec DER DC Measurement Model (Model 714) - 4x PV ports + 1x ESS port"""
    
    # Model header
    model_id: int = 714
    model_length: int = 0  # Will be calculated based on number of ports
    
    # General DC measurements
    port_alarms: int = 0  # PrtAlrms - Bitfield of ports with active alarms
    num_ports: int = 5  # NPrt - Number of DC ports (4 PV + 1 ESS)
    total_dc_current: float = 0.0  # DCA - Total DC current for all ports
    total_dc_power: float = 0.0  # DCW - Total DC power for all ports
    total_dc_energy_injected: int = 0  # DCWhInj - Total cumulative DC energy injected
    total_dc_energy_absorbed: int = 0  # DCWhAbs - Total cumulative DC energy absorbed
    
    # Scale factors
    current_sf: int = -2  # DCA_SF - DC current scale factor
    voltage_sf: int = -1  # DCV_SF - DC voltage scale factor
    power_sf: int = 0  # DCW_SF - DC power scale factor
    energy_sf: int = -3  # DCWH_SF - DC energy scale factor
    temperature_sf: int = -1  # Tmp_SF - Temperature scale factor
    
    # DC Ports (4 PV + 1 ESS)
    ports: list = field(default_factory=lambda: [
        SunSpecDCPort(port_type=0, port_id=1, port_id_string="MPPT1"),      # PV Port 1
        SunSpecDCPort(port_type=0, port_id=2, port_id_string="MPPT2"),      # PV Port 2
        SunSpecDCPort(port_type=0, port_id=3, port_id_string="MPPT3"),      # PV Port 3
        SunSpecDCPort(port_type=0, port_id=4, port_id_string="MPPT"),      # PV Port 4 (uninitialized)
        SunSpecDCPort(port_type=1, port_id=5, port_id_string="BATT1")     # Battery Port 1
    ])
    
    def __post_init__(self):
        """Calculate model length based on number of ports"""
        # Base model: 2 (header) + 2 (PrtAlrms) + 1 (NPrt) + 1 (DCA) + 1 (DCW) + 4 (DCWhInj) + 4 (DCWhAbs) + 5 (scale factors) = 20
        # Per port: 1 (PrtTyp) + 1 (ID) + 8 (IDStr) + 1 (DCA) + 1 (DCV) + 1 (DCW) + 4 (DCWhInj) + 4 (DCWhAbs) + 1 (Tmp) + 1 (DCSta) + 2 (DCAlrm) = 25
        base_length = 18  # Without header
        port_length = 25
        self.model_length = base_length + (self.num_ports * port_length)


class SunSpecRegisterMap:
    """SunSpec register mapping for Modbus TCP server with dual 701 instances and 714 model"""
    
    # Base addresses for each model
    SUNSPEC_BASE_ADDR = 40000
    COMMON_MODEL_BASE = SUNSPEC_BASE_ADDR + 2  # 40002 (after SunS header)
    GRID_MODEL_BASE = COMMON_MODEL_BASE + 66 + 2  # 40070 (after Common Model + header)
    LOAD_MODEL_BASE = GRID_MODEL_BASE + 153 + 2  # 40225 (after Grid Model + header)
    STORAGE_MODEL_BASE = LOAD_MODEL_BASE + 153 + 2  # 40380 (after Load Model + header)
    DC_MODEL_BASE = STORAGE_MODEL_BASE + 7 + 2  # 40389 (after Storage Model + header)
    END_MODEL_BASE = DC_MODEL_BASE + 143 + 2  # 40534 (after DC Model + header, 143 = 18 base + 5*25 ports)
    
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
    
    # Grid Model (701) registers - First instance
    GRID_MODEL_ID = GRID_MODEL_BASE  # 40070
    GRID_MODEL_LENGTH = GRID_MODEL_BASE + 1  # 40071
    GRID_AC_TYPE = GRID_MODEL_BASE + 2  # 40072
    GRID_OPERATING_STATE = GRID_MODEL_BASE + 3  # 40073
    GRID_STATUS = GRID_MODEL_BASE + 4  # 40074
    GRID_CONNECTION = GRID_MODEL_BASE + 5  # 40075
    GRID_ALARM = GRID_MODEL_BASE + 6  # 40076-40077 (2 registers)
    GRID_DER_MODE = GRID_MODEL_BASE + 8  # 40078-40079 (2 registers)
    GRID_AC_POWER = GRID_MODEL_BASE + 10  # 40080
    GRID_AC_VA = GRID_MODEL_BASE + 11  # 40081
    GRID_AC_VAR = GRID_MODEL_BASE + 12  # 40082
    GRID_AC_PF = GRID_MODEL_BASE + 13  # 40083
    GRID_AC_CURRENT = GRID_MODEL_BASE + 14  # 40084
    GRID_AC_VOLTAGE_LL = GRID_MODEL_BASE + 15  # 40085
    GRID_AC_VOLTAGE_LN = GRID_MODEL_BASE + 16  # 40086
    GRID_AC_FREQUENCY = GRID_MODEL_BASE + 17  # 40087-40088 (2 registers)
    
    # Load Model (701) registers - Second instance
    LOAD_MODEL_ID = LOAD_MODEL_BASE  # 40225
    LOAD_MODEL_LENGTH = LOAD_MODEL_BASE + 1  # 40226
    LOAD_AC_TYPE = LOAD_MODEL_BASE + 2  # 40227
    LOAD_OPERATING_STATE = LOAD_MODEL_BASE + 3  # 40228
    LOAD_STATUS = LOAD_MODEL_BASE + 4  # 40229
    LOAD_CONNECTION = LOAD_MODEL_BASE + 5  # 40230
    LOAD_ALARM = LOAD_MODEL_BASE + 6  # 40231-40232 (2 registers)
    LOAD_DER_MODE = LOAD_MODEL_BASE + 8  # 40233-40234 (2 registers)
    LOAD_AC_POWER = LOAD_MODEL_BASE + 10  # 40235
    LOAD_AC_VA = LOAD_MODEL_BASE + 11  # 40236
    LOAD_AC_VAR = LOAD_MODEL_BASE + 12  # 40237
    LOAD_AC_PF = LOAD_MODEL_BASE + 13  # 40238
    LOAD_AC_CURRENT = LOAD_MODEL_BASE + 14  # 40239
    LOAD_AC_VOLTAGE_LL = LOAD_MODEL_BASE + 15  # 40240
    LOAD_AC_VOLTAGE_LN = LOAD_MODEL_BASE + 16  # 40241
    LOAD_AC_FREQUENCY = LOAD_MODEL_BASE + 17  # 40242-40243 (2 registers)
    
    # Storage Model (713) registers
    STORAGE_MODEL_ID = STORAGE_MODEL_BASE + 0  # 40380
    STORAGE_MODEL_LENGTH = STORAGE_MODEL_BASE + 1  # 40381
    STORAGE_ENERGY_RATING = STORAGE_MODEL_BASE + 2  # 40382
    STORAGE_ENERGY_AVAILABLE = STORAGE_MODEL_BASE + 3  # 40383
    STORAGE_SOC = STORAGE_MODEL_BASE + 4  # 40384
    STORAGE_SOH = STORAGE_MODEL_BASE + 5  # 40385
    STORAGE_STATUS = STORAGE_MODEL_BASE + 6  # 40386
    STORAGE_SF_ENERGY = STORAGE_MODEL_BASE + 7  # 40387
    STORAGE_SF_PERCENT = STORAGE_MODEL_BASE + 8  # 40388
    
    # DC Model (714) registers
    DC_MODEL_ID = DC_MODEL_BASE + 0  # 40389
    DC_MODEL_LENGTH = DC_MODEL_BASE + 1  # 40390
    DC_PORT_ALARMS = DC_MODEL_BASE + 2  # 40391-40392 (2 registers for bitfield32)
    DC_NUM_PORTS = DC_MODEL_BASE + 4  # 40393
    DC_TOTAL_CURRENT = DC_MODEL_BASE + 5  # 40394
    DC_TOTAL_POWER = DC_MODEL_BASE + 6  # 40395
    DC_TOTAL_ENERGY_INJ = DC_MODEL_BASE + 7  # 40396-40399 (4 registers for uint64)
    DC_TOTAL_ENERGY_ABS = DC_MODEL_BASE + 11  # 40400-40403 (4 registers for uint64)
    DC_CURRENT_SF = DC_MODEL_BASE + 15  # 40404
    DC_VOLTAGE_SF = DC_MODEL_BASE + 16  # 40405
    DC_POWER_SF = DC_MODEL_BASE + 17  # 40406
    DC_ENERGY_SF = DC_MODEL_BASE + 18  # 40407
    DC_TEMP_SF = DC_MODEL_BASE + 19  # 40408
    
    # DC Port base addresses (each port takes 25 registers)
    DC_PORT1_BASE = DC_MODEL_BASE + 20  # 40409 - PV Port 1
    DC_PORT2_BASE = DC_PORT1_BASE + 25  # 40434 - PV Port 2
    DC_PORT3_BASE = DC_PORT2_BASE + 25  # 40459 - PV Port 3
    DC_PORT4_BASE = DC_PORT3_BASE + 25  # 40484 - PV Port 4 (uninitialized)
    DC_PORT5_BASE = DC_PORT4_BASE + 25  # 40509 - ESS Port 1
    
    # End-of-map marker
    END_MODEL_ID = END_MODEL_BASE + 0  # 40534 - Model ID 65535 (0xFFFF)
    END_MODEL_LENGTH = END_MODEL_BASE + 1  # 40535 - Length 0


class SunSpecMapper:
    """Maps inverter data to SunSpec models with dual 701 instances"""
    
    # Constants for register initialization
    NULL_UINT16 = 0xFFFF  # Not implemented value for unsigned 16-bit
    NULL_INT16 = 0x8000   # Not implemented value for signed 16-bit
    
    # Register offset groups for signed int16 values that need NULL_INT16
    GRID_SIGNED_OFFSETS = [35, 36, 39, 40, 42, 43, 44, 65, 66, 67, 87, 88, 89, 90, 91]
    LOAD_SIGNED_OFFSETS = [35, 36, 39, 40, 42, 43, 44, 65, 66, 67, 87, 88, 89, 90, 91]
    
    # Register descriptions for better maintainability
    REGISTER_DESCRIPTIONS = {
        35: "TmpAmb - Ambient Temperature",
        36: "TmpCab - Cabinet Temperature",
        39: "TmpSw - Switch Temperature",
        40: "TmpOt - Other Temperature",
        42: "VAL1",
        43: "VarL1",
        44: "PFL1",
        65: "VAL2",
        66: "VarL2",
        67: "PFL2",
        87: "WL3",
        88: "VAL3",
        90: "VarL3",
        89: "PFL3",
        91: "AL3"
    }
    
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
        
        # Initialize dual 701 models
        self.grid_model = SunSpecGridModel()
        self.load_model = SunSpecLoadModel()
        self.battery_model = SunSpecBatteryModel()
        self.dc_model = SunSpec714Model()
        
        # Register map for Modbus server
        self.registers = {}
        self._initialize_registers()
    
    def _initialize_registers(self):
        """Initialize the Modbus register map with dual 701 instances"""
        
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
        
        ###############################################
        # Grid Model (701) header - First instance
        ###############################################
        self._set_register(SunSpecRegisterMap.GRID_MODEL_ID, self.grid_model.model_id)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_LENGTH, self.grid_model.model_length)
        
        # Initialize all grid model values to "not implemented" (0xFFFF)
        for i in range(2, self.grid_model.model_length + 2):  # +2 for header
            self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + i, 0xFFFF)
        
        # Initialize 701 model signed int values to "not implemented" using bulk operation
        self._set_signed_registers_to_null(SunSpecRegisterMap.GRID_MODEL_BASE, self.GRID_SIGNED_OFFSETS)
        
        # Set scale factors for Grid model
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 113, -2)  # Current scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 114, -1)  # Voltage scale factor: -1 (0.1)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 115, -2)  # Frequency scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 116, 0)   # Power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 117, -2)  # Power factor scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 118, 0)   # Apparent power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 119, 0)   # Reactive power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 120, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 121, -3)  # Reactive energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 122, -1)  # Temperature scale factor: -1 (0.1)

        ###############################################
        # Load Model (701) header - Second instance
        ###############################################
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_ID, self.load_model.model_id)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_LENGTH, self.load_model.model_length)
        
        # Initialize all load model values to "not implemented" (0xFFFF)
        for i in range(2, self.load_model.model_length + 2):  # +2 for header
            self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + i, 0xFFFF)
        
        # Initialize 701 model signed int values to "not implemented" using bulk operation
        self._set_signed_registers_to_null(SunSpecRegisterMap.LOAD_MODEL_BASE, self.LOAD_SIGNED_OFFSETS)
        
        # Set scale factors for Load model
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 113, -2)  # Current scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 114, -1)  # Voltage scale factor: -1 (0.1)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 115, -2)  # Frequency scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 116, 0)   # Power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 117, -2)  # Power factor scale factor: -2 (0.01)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 118, 0)   # Apparent power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 119, 0)   # Reactive power scale factor: 0 (1)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 120, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 121, -3)  # Reactive energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 122, -1)  # Temperature scale factor: -1 (0.1)
        
        ###############################################
        # Storage Model (713)
        ###############################################
        # Storage Model header
        self._set_register(SunSpecRegisterMap.STORAGE_MODEL_ID, self.battery_model.model_id)
        self._set_register(SunSpecRegisterMap.STORAGE_MODEL_LENGTH, self.battery_model.model_length)
        
        # Initialize all storage model values to "not implemented" (0xFFFF)
        for i in range(2, self.battery_model.model_length + 2):  # +2 for header
            self._set_register(SunSpecRegisterMap.STORAGE_MODEL_BASE + i, 0xFFFF)
        
        # Set scale factors for storage model
        self._set_register(SunSpecRegisterMap.STORAGE_SF_ENERGY, -3)  # Energy scale factor: -3 (0.001)
        self._set_register(SunSpecRegisterMap.STORAGE_SF_PERCENT, -1)  # Percentage scale factor: -1 (0.1)
        
        ###############################################
        # DC Measurement Model (714)
        ###############################################
        # DC Model (714) header
        self._set_register(SunSpecRegisterMap.DC_MODEL_ID, self.dc_model.model_id)
        self._set_register(SunSpecRegisterMap.DC_MODEL_LENGTH, self.dc_model.model_length)
        
        # Initialize DC model general registers
        self._set_register_32bit(SunSpecRegisterMap.DC_PORT_ALARMS, 0)  # No port alarms initially
        self._set_register(SunSpecRegisterMap.DC_NUM_PORTS, self.dc_model.num_ports)
        self._set_register(SunSpecRegisterMap.DC_TOTAL_CURRENT, 0)  # Will be updated with real data
        self._set_register(SunSpecRegisterMap.DC_TOTAL_POWER, 0)  # Will be updated with real data
        
        # Initialize total energy registers (64-bit values)
        for i in range(4):
            self._set_register(SunSpecRegisterMap.DC_TOTAL_ENERGY_INJ + i, 0)
            self._set_register(SunSpecRegisterMap.DC_TOTAL_ENERGY_ABS + i, 0)
        
        # Set scale factors for DC model
        self._set_register(SunSpecRegisterMap.DC_CURRENT_SF, self.dc_model.current_sf)
        self._set_register(SunSpecRegisterMap.DC_VOLTAGE_SF, self.dc_model.voltage_sf)
        self._set_register(SunSpecRegisterMap.DC_POWER_SF, self.dc_model.power_sf)
        self._set_register(SunSpecRegisterMap.DC_ENERGY_SF, self.dc_model.energy_sf)
        self._set_register(SunSpecRegisterMap.DC_TEMP_SF, self.dc_model.temperature_sf)
        
        # Initialize DC ports
        port_bases = [SunSpecRegisterMap.DC_PORT1_BASE, SunSpecRegisterMap.DC_PORT2_BASE,
                     SunSpecRegisterMap.DC_PORT3_BASE, SunSpecRegisterMap.DC_PORT4_BASE,
                     SunSpecRegisterMap.DC_PORT5_BASE]
        
        for i, port_base in enumerate(port_bases):
            port = self.dc_model.ports[i]
            
            # Port type and ID
            self._set_register(port_base + 0, port.port_type)  # PrtTyp
            self._set_register(port_base + 1, port.port_id)    # ID
            
            # Port ID string (8 registers)
            self._set_string_registers(port_base + 2, port.port_id_string, 8)
            
            # Initialize port measurements (will be updated with real data)
            if i == 3:  # Port 4 (PV4) - set as uninitialized
                self._set_register(port_base + 10, 0xFFFF)  # DCA - uninitialized
                self._set_register(port_base + 11, 0xFFFF)  # DCV - uninitialized
                self._set_register(port_base + 12, 0xFFFF)  # DCW - uninitialized
                self._set_register(port_base + 21, 0x8000)  # Tmp - uninitialized (signed)
                self._set_register(port_base + 22, 0xFFFF)  # DCSta - uninitialized
            else:
                self._set_register(port_base + 10, 0)  # DCA
                self._set_register(port_base + 11, 0)  # DCV
                self._set_register(port_base + 12, 0)  # DCW
                self._set_register(port_base + 21, 0)  # Tmp
                self._set_register(port_base + 22, 0)  # DCSta - OFF
            
            # Initialize energy registers (64-bit values)
            for j in range(4):
                self._set_register(port_base + 13 + j, 0)  # DCWhInj (4 registers)
                self._set_register(port_base + 17 + j, 0)  # DCWhAbs (4 registers)
            
            # Initialize alarm register (32-bit)
            self._set_register_32bit(port_base + 23, 0)  # DCAlrm
        
        ###############################################
        # End-of-map marker
        ###############################################
        self._set_register(SunSpecRegisterMap.END_MODEL_ID, 0xFFFF)  # End marker
        self._set_register(SunSpecRegisterMap.END_MODEL_LENGTH, 0)   # Length 0
    
    def _set_register(self, address, value):
        """Set a single 16-bit register"""
        self.registers[address] = int(value) & 0xFFFF
    
    def _set_register_32bit(self, address, value):
        """Set a 32-bit value across two registers (big-endian)"""
        self.registers[address] = (int(value) >> 16) & 0xFFFF      # High word
        self.registers[address + 1] = int(value) & 0xFFFF          # Low word
    
    def _set_string_registers(self, base_address, text, num_registers):
        """Set string value across multiple registers (2 chars per register)"""
        # Ensure text is a string and handle None values
        if text is None:
            text = ""
        text = str(text)
        
        # Truncate if too long, then pad with null bytes to fill exactly the allocated space
        max_chars = num_registers * 2
        if len(text) >= max_chars:
            # If text is too long, truncate and ensure null termination
            padded_text = text[:max_chars-1] + '\0'
        else:
            # If text fits, null-terminate and pad with null bytes
            padded_text = text + '\0' + '\0' * (max_chars - len(text) - 1)
        
        # Ensure we have exactly the right number of characters
        padded_text = padded_text[:max_chars]
        
        for i in range(num_registers):
            char1 = ord(padded_text[i * 2]) if i * 2 < len(padded_text) else 0
            char2 = ord(padded_text[i * 2 + 1]) if i * 2 + 1 < len(padded_text) else 0
            self.registers[base_address + i] = (char1 << 8) | char2
    
    def _set_signed_registers_to_null(self, base_address, offsets):
        """Set multiple signed registers to NULL_INT16 value"""
        for offset in offsets:
            self.registers[base_address + offset] = self.NULL_INT16
    
    def update_from_inverter_data(self, inverter_data: InverterData):
        """
        Update SunSpec models from inverter data
        
        Args:
            inverter_data: InverterData instance containing current measurements
        """
        try:
            # Update Grid Model (701) - First instance
            self._update_grid_model(inverter_data)
            
            # Update Load Model (701) - Second instance
            self._update_load_model(inverter_data)
            
            # Update Storage Model (713)
            self._update_storage_model(inverter_data)
            
            # Update DC Model (714)
            self._update_dc_model(inverter_data)
            
            self.logger.debug("SunSpec models updated successfully")
            
        except Exception as e:
            self.logger.error(f"Error updating SunSpec models: {e}")
    
    def _update_grid_model(self, data: InverterData):
        """Update Grid Model (701) registers"""
        base = SunSpecRegisterMap.GRID_MODEL_BASE
        
        # Determine AC Type based on grid type
        ac_type = 101  # Single-phase default
        if hasattr(data, 'grid_type'):
            if data.grid_type == 1:
                ac_type = 102  # Split-phase
            elif data.grid_type == 2:
                ac_type = 103  # Three-phase Wye
        elif hasattr(data, 'phase_type'):
            if data.phase_type == 'split_phase':
                ac_type = 102
            elif data.phase_type == 'three_phase':
                ac_type = 103
        
        self._set_register(base + 2, ac_type)
        
        # Operating state (4 = MPPT, 5 = Throttled, 7 = Shutting down, 8 = Fault)
        self._set_register(base + 3, 4)  # Default to MPPT
        
        # Status and connection
        self._set_register(base + 4, 1)  # Status: Connected
        self._set_register(base + 5, 1)  # Connection: Connected
        
        # AC measurements with scaling
        if hasattr(data, 'grid_power') and data.grid_power is not None:
            self._set_register(base + 10, int(data.grid_power))  # AC Power (W)
        
        # Line-to-line voltages
        if hasattr(data, 'grid_voltage_l1l2') and data.grid_voltage_l1l2 is not None:
            self._set_register(base + 15, int(data.grid_voltage_l1l2 * 10))  # AC Voltage L1-L2
        elif hasattr(data, 'grid_voltage') and data.grid_voltage is not None:
            # Fallback to legacy grid_voltage
            self._set_register(base + 15, int(data.grid_voltage * 10))  # AC Voltage LL
        
        # Line-to-neutral voltages
        if hasattr(data, 'grid_voltage_l1n') and data.grid_voltage_l1n is not None:
            self._set_register(base + 16, int(data.grid_voltage_l1n * 10))  # AC Voltage L1-N
        
        # Phase currents
        if hasattr(data, 'grid_current_l1') and data.grid_current_l1 is not None:
            self._set_register(base + 14, int(data.grid_current_l1 * 100))  # AC Current L1
        
        # For three-phase systems, add L2 and L3 measurements
        if ac_type == 103:  # Three-phase
            # L2 measurements
            if hasattr(data, 'grid_voltage_l2n') and data.grid_voltage_l2n is not None:
                self._set_register(base + 45, int(data.grid_voltage_l2n * 10))  # VL2N
            if hasattr(data, 'grid_current_l2') and data.grid_current_l2 is not None:
                self._set_register(base + 46, int(data.grid_current_l2 * 100))  # AL2
            if hasattr(data, 'grid_power_l2') and data.grid_power_l2 is not None:
                self._set_register(base + 47, int(data.grid_power_l2))  # WL2
            
            # L3 measurements
            if hasattr(data, 'grid_voltage_l3n') and data.grid_voltage_l3n is not None:
                self._set_register(base + 65, int(data.grid_voltage_l3n * 10))  # VL3N
            if hasattr(data, 'grid_current_l3') and data.grid_current_l3 is not None:
                self._set_register(base + 66, int(data.grid_current_l3 * 100))  # AL3
            if hasattr(data, 'grid_power_l3') and data.grid_power_l3 is not None:
                self._set_register(base + 67, int(data.grid_power_l3))  # WL3
            
            # Additional three-phase line-to-line voltages
            if hasattr(data, 'grid_voltage_l2l3') and data.grid_voltage_l2l3 is not None:
                self._set_register(base + 68, int(data.grid_voltage_l2l3 * 10))  # VL2L3
            if hasattr(data, 'grid_voltage_l3l1') and data.grid_voltage_l3l1 is not None:
                self._set_register(base + 69, int(data.grid_voltage_l3l1 * 10))  # VL3L1
        
        if hasattr(data, 'grid_frequency') and data.grid_frequency is not None:
            # Frequency with scale factor -2 (0.01Hz resolution)
            freq_32bit = int(data.grid_frequency * 100)
            self._set_register_32bit(base + 17, freq_32bit)  # AC Frequency
        
        # Temperature
        if hasattr(data, 'inverter_temperature') and data.inverter_temperature is not None:
            # Temperature with scale factor -1 (0.1°C resolution)
            self._set_register(base + 36, int(data.inverter_temperature * 10))  # Cabinet Temperature
        elif hasattr(data, 'igbt_temp') and data.igbt_temp is not None:
            # Use IGBT temperature as fallback
            self._set_register(base + 36, int(data.igbt_temp * 10))  # Cabinet Temperature

    def _update_load_model(self, data: InverterData):
        """Update Load Model (701) registers"""
        base = SunSpecRegisterMap.LOAD_MODEL_BASE
        
        # Determine AC Type based on grid type
        ac_type = 101  # Single-phase default
        if hasattr(data, 'grid_type'):
            if data.grid_type == 1:
                ac_type = 102  # Split-phase
            elif data.grid_type == 2:
                ac_type = 103  # Three-phase Wye
        elif hasattr(data, 'phase_type'):
            if data.phase_type == 'split_phase':
                ac_type = 102
            elif data.phase_type == 'three_phase':
                ac_type = 103
        
        self._set_register(base + 2, ac_type)
        
        # Operating state
        self._set_register(base + 3, 4)  # Default to MPPT
        
        # Status and connection
        self._set_register(base + 4, 1)  # Status: Connected
        self._set_register(base + 5, 1)  # Connection: Connected
        
        # AC measurements with scaling
        if hasattr(data, 'load_power_total') and data.load_power_total is not None:
            self._set_register(base + 10, int(data.load_power_total))  # AC Power (W)
        
        # Line-to-line voltages
        if hasattr(data, 'load_voltage_l1l2') and data.load_voltage_l1l2 is not None:
            self._set_register(base + 15, int(data.load_voltage_l1l2 * 10))  # AC Voltage L1-L2
        
        # Line-to-neutral voltages
        if hasattr(data, 'load_voltage_l1n') and data.load_voltage_l1n is not None:
            self._set_register(base + 16, int(data.load_voltage_l1n * 10))  # AC Voltage L1-N
        
        # Phase currents
        if hasattr(data, 'load_current_l1') and data.load_current_l1 is not None:
            self._set_register(base + 14, int(data.load_current_l1 * 100))  # AC Current L1
        
        # For three-phase systems, add L2 and L3 measurements
        if ac_type == 103:  # Three-phase
            # L2 measurements
            if hasattr(data, 'load_voltage_l2n') and data.load_voltage_l2n is not None:
                self._set_register(base + 45, int(data.load_voltage_l2n * 10))  # VL2N
            if hasattr(data, 'load_current_l2') and data.load_current_l2 is not None:
                self._set_register(base + 46, int(data.load_current_l2 * 100))  # AL2
            if hasattr(data, 'load_power_l2') and data.load_power_l2 is not None:
                self._set_register(base + 47, int(data.load_power_l2))  # WL2
            
            # L3 measurements
            if hasattr(data, 'load_voltage_l3n') and data.load_voltage_l3n is not None:
                self._set_register(base + 65, int(data.load_voltage_l3n * 10))  # VL3N
            if hasattr(data, 'load_current_l3') and data.load_current_l3 is not None:
                self._set_register(base + 66, int(data.load_current_l3 * 100))  # AL3
            if hasattr(data, 'load_power_l3') and data.load_power_l3 is not None:
                self._set_register(base + 67, int(data.load_power_l3))  # WL3
            
            # Additional three-phase line-to-line voltages
            if hasattr(data, 'load_voltage_l2l3') and data.load_voltage_l2l3 is not None:
                self._set_register(base + 68, int(data.load_voltage_l2l3 * 10))  # VL2L3
            if hasattr(data, 'load_voltage_l3l1') and data.load_voltage_l3l1 is not None:
                self._set_register(base + 69, int(data.load_voltage_l3l1 * 10))  # VL3L1
        
        if hasattr(data, 'load_frequency') and data.load_frequency is not None:
            # Frequency with scale factor -2 (0.01Hz resolution)
            freq_32bit = int(data.load_frequency * 100)
            self._set_register_32bit(base + 17, freq_32bit)  # AC Frequency
    
    def _update_storage_model(self, data: InverterData):
        """Update Storage Model (713) registers"""
        base = SunSpecRegisterMap.STORAGE_MODEL_BASE
        
        # Battery measurements
        if hasattr(data, 'battery_soc') and data.battery_soc is not None:
            # SoC with scale factor -1 (0.1% resolution)
            self._set_register(base + 4, int(data.battery_soc * 10))
        
        if hasattr(data, 'battery_capacity') and data.battery_capacity is not None:
            # Energy rating with scale factor -3 (0.001 kWh resolution)
            self._set_register(base + 2, int(data.battery_capacity * 1000))
        
        # Battery status (0=OFF, 1=EMPTY, 2=DISCHARGING, 3=CHARGING, 4=FULL, 5=HOLDING, 6=TESTING)
        if hasattr(data, 'battery_power') and data.battery_power is not None:
            if data.battery_power > 50:
                status = 3  # Charging
            elif data.battery_power < -50:
                status = 2  # Discharging
            else:
                status = 5  # Holding
            self._set_register(base + 6, status)
    
    def _update_dc_model(self, data: InverterData):
        """Update DC Model (714) registers"""
        base = SunSpecRegisterMap.DC_MODEL_BASE
        
        # Update total DC measurements
        total_current = 0.0
        total_power = 0.0
        
        # Update PV ports (ports 1-3, port 4 uninitialized)
        pv_powers = []
        if hasattr(data, 'pv1_power') and data.pv1_power is not None:
            pv_powers.append(data.pv1_power)
        if hasattr(data, 'pv2_power') and data.pv2_power is not None:
            pv_powers.append(data.pv2_power)
        if hasattr(data, 'pv3_power') and data.pv3_power is not None:
            pv_powers.append(data.pv3_power)
        
        pv_voltages = []
        if hasattr(data, 'pv1_voltage') and data.pv1_voltage is not None:
            pv_voltages.append(data.pv1_voltage)
        if hasattr(data, 'pv2_voltage') and data.pv2_voltage is not None:
            pv_voltages.append(data.pv2_voltage)
        if hasattr(data, 'pv3_voltage') and data.pv3_voltage is not None:
            pv_voltages.append(data.pv3_voltage)
        
        # Update individual PV ports
        port_bases = [SunSpecRegisterMap.DC_PORT1_BASE, SunSpecRegisterMap.DC_PORT2_BASE, SunSpecRegisterMap.DC_PORT3_BASE]
        
        for i, port_base in enumerate(port_bases):
            if i < len(pv_powers) and i < len(pv_voltages):
                power = pv_powers[i]
                voltage = pv_voltages[i]
                current = power / voltage if voltage > 0 else 0
                
                # Update port measurements with scaling
                self._set_register(port_base + 10, int(current * 100))  # DCA (scale -2)
                self._set_register(port_base + 11, int(voltage * 10))   # DCV (scale -1)
                self._set_register(port_base + 12, int(power))          # DCW (scale 0)
                self._set_register(port_base + 22, 1)                   # DCSta - ON
                
                total_current += current
                total_power += power
        
        # Update battery port (port 5)
        if hasattr(data, 'battery_power') and data.battery_power is not None:
            battery_power = data.battery_power
            battery_voltage = getattr(data, 'battery_voltage', 48.0)  # Default 48V
            battery_current = battery_power / battery_voltage if battery_voltage > 0 else 0
            
            port_base = SunSpecRegisterMap.DC_PORT5_BASE
            self._set_register(port_base + 10, int(battery_current * 100))  # DCA (scale -2)
            self._set_register(port_base + 11, int(battery_voltage * 10))   # DCV (scale -1)
            self._set_register(port_base + 12, int(battery_power))          # DCW (scale 0)
            self._set_register(port_base + 22, 1)                           # DCSta - ON
            
            total_current += abs(battery_current)  # Use absolute value for total
            total_power += abs(battery_power)      # Use absolute value for total
        
        # Update totals
        self._set_register(SunSpecRegisterMap.DC_TOTAL_CURRENT, int(total_current * 100))  # Scale -2
        self._set_register(SunSpecRegisterMap.DC_TOTAL_POWER, int(total_power))            # Scale 0
    
    def get_registers(self):
        """Get the complete register map for Modbus server"""
        return self.registers.copy()
    
    def get_all_registers(self):
        """Get the complete register map for Modbus server (alias for compatibility)"""
        return self.get_registers()
    
    def get_register_range(self, start_addr, count):
        """Get a range of registers for Modbus server"""
        result = []
        for addr in range(start_addr, start_addr + count):
            result.append(self.registers.get(addr, 0))
        return result
    
    def log_register_map(self):
        """Log the current register map for debugging"""
        self.logger.info("SunSpec Register Map:")
        self.logger.info(f"  SunSpec ID: {self.registers.get(SunSpecRegisterMap.SUNSPEC_ID, 0):04X}")
        self.logger.info(f"  Common Model: {self.registers.get(SunSpecRegisterMap.COMMON_MODEL_ID, 0)}")
        self.logger.info(f"  Grid Model: {self.registers.get(SunSpecRegisterMap.GRID_MODEL_ID, 0)}")
        self.logger.info(f"  Load Model: {self.registers.get(SunSpecRegisterMap.LOAD_MODEL_ID, 0)}")
        self.logger.info(f"  Storage Model: {self.registers.get(SunSpecRegisterMap.STORAGE_MODEL_ID, 0)}")
        self.logger.info(f"  DC Model: {self.registers.get(SunSpecRegisterMap.DC_MODEL_ID, 0)}")
        self.logger.info(f"  End Marker: {self.registers.get(SunSpecRegisterMap.END_MODEL_ID, 0):04X}")