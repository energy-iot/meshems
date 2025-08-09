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
    pass


@dataclass
class SunSpecLoadModel(SunSpecInverterModel):
    """SunSpec Load Model (Model 701) - Load-side measurements"""
    pass


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
    """Maps Sol-Ark data to SunSpec models with dual 701 instances"""
    
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
                if i == 3:  # Port 4 - uninitialized
                    self._set_register(port_base + 13 + j, 0xFFFF)  # DCWhInj
                    self._set_register(port_base + 17 + j, 0xFFFF)  # DCWhAbs
                else:
                    self._set_register(port_base + 13 + j, 0)  # DCWhInj
                    self._set_register(port_base + 17 + j, 0)  # DCWhAbs
            
            # Initialize alarm register (32-bit)
            if i == 3:  # Port 4 - uninitialized
                self._set_register_32bit(port_base + 23, 0xFFFFFFFF)  # DCAlrm
            else:
                self._set_register_32bit(port_base + 23, 0)  # DCAlrm
        
        # End-of-map marker (Model ID 65535, Length 0)
        self._set_register(SunSpecRegisterMap.END_MODEL_ID, 65535)  # 0xFFFF
        self._set_register(SunSpecRegisterMap.END_MODEL_LENGTH, 0)
    
    def _set_register(self, address, value):
        """Set a single register value with validation"""
        if not isinstance(address, int) or address < 0:
            raise ValueError(f"Invalid register address: {address}")
        if not isinstance(value, (int, float)):
            raise ValueError(f"Invalid register value type: {type(value)}")
        
        # Ensure value fits in 16-bit register
        self.registers[address] = int(value) & 0xFFFF
    
    def _set_signed_registers_to_null(self, base_address, offsets):
        """
        Set multiple signed registers to NULL_INT16 value in bulk
        
        Args:
            base_address: Base register address
            offsets: List of register offsets from base address
        """
        try:
            for offset in offsets:
                register_addr = base_address + offset
                self._set_register(register_addr, self.NULL_INT16)
                
                # Log register initialization for debugging
                description = self.REGISTER_DESCRIPTIONS.get(offset, f"Register {offset}")
                self.logger.debug(f"Initialized {description} at address {register_addr} to NULL_INT16")
                
        except Exception as e:
            self.logger.error(f"Error setting signed registers to null: {e}")
            raise
    
    def _set_registers_bulk(self, register_map):
        """
        Set multiple registers from a dictionary mapping
        
        Args:
            register_map: Dictionary of {address: value} pairs
        """
        try:
            for address, value in register_map.items():
                self._set_register(address, value)
        except Exception as e:
            self.logger.error(f"Error in bulk register setting: {e}")
            raise
    
    def _validate_register_range(self, base_address, offsets, model_length):
        """
        Validate that register offsets are within model bounds
        
        Args:
            base_address: Base register address
            offsets: List of register offsets
            model_length: Maximum model length
        """
        for offset in offsets:
            if offset >= model_length:
                raise ValueError(f"Register offset {offset} exceeds model length {model_length}")
    
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
            # Update Grid model with grid-side measurements
            self.grid_model.ac_current = abs(solark_data.grid_current_l1 + solark_data.grid_current_l2)
            self.grid_model.ac_current_a = solark_data.grid_current_l1
            self.grid_model.ac_current_b = solark_data.grid_current_l2
            self.grid_model.ac_voltage_ab = solark_data.grid_voltage_l1l2
            self.grid_model.ac_power = solark_data.grid_power
            self.grid_model.ac_frequency = solark_data.grid_frequency
            self.grid_model.ac_energy = solark_data.grid_sell_energy * 1000  # Convert kWh to Wh
            
            # Update Load model with load-side measurements
            self.load_model.ac_current = abs(solark_data.load_current_l1 + solark_data.load_current_l2)
            self.load_model.ac_current_a = solark_data.load_current_l1
            self.load_model.ac_current_b = solark_data.load_current_l2
            self.load_model.ac_voltage_ab = solark_data.inverter_voltage  # Load voltage from inverter side
            self.load_model.ac_power = solark_data.load_power_total
            self.load_model.ac_frequency = solark_data.load_frequency
            self.load_model.ac_energy = solark_data.load_energy * 1000  # Convert kWh to Wh
            
            # DC measurements (battery side) - shared between models
            dc_current = abs(solark_data.battery_current)
            dc_voltage = solark_data.battery_voltage
            dc_power = abs(solark_data.battery_power)
            
            self.grid_model.dc_current = dc_current
            self.grid_model.dc_voltage = dc_voltage
            self.grid_model.dc_power = dc_power
            
            self.load_model.dc_current = dc_current
            self.load_model.dc_voltage = dc_voltage
            self.load_model.dc_power = dc_power
            
            # Temperature - shared between models
            # cabinet_temp = 0x8000  # Not available
            # self.grid_model.cabinet_temperature = cabinet_temp
            # self.load_model.cabinet_temperature = cabinet_temp
            
            # Operating state mapping - shared between models
            operating_state = self._map_inverter_state(solark_data.inverter_status)
            vendor_state = self._map_vendor_state(solark_data)
            
            self.grid_model.operating_state = operating_state
            self.grid_model.vendor_operating_state = vendor_state
            self.load_model.operating_state = operating_state
            self.load_model.vendor_operating_state = vendor_state
            
            # Update battery model
            self.battery_model.battery_voltage = solark_data.battery_voltage
            self.battery_model.battery_current = solark_data.battery_current
            self.battery_model.battery_power = solark_data.battery_power
            self.battery_model.battery_soc = solark_data.battery_soc
            self.battery_model.battery_temperature = solark_data.battery_temperature
            self.battery_model.battery_capacity = solark_data.battery_capacity
            self.battery_model.battery_energy_capacity = solark_data.battery_capacity * solark_data.battery_voltage
            self.battery_model.battery_status = self._map_storage_status(solark_data)
            
            # Update Modbus registers for both models
            self.update_grid_registers_from_solark(solark_data)
            self.update_load_registers_from_solark(solark_data)
            self._update_battery_registers()
            self._update_dc_model_from_solark(solark_data)
            
            # Legacy compatibility - use grid registers function
            # (Legacy inverter model is an alias to grid model)
            
            self.logger.debug("Updated SunSpec Grid, Load, and DC models with Sol-Ark data")
            
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
    

    ###############################################
    # Grid Model (701) header - First instance
    ###############################################    
    def update_grid_registers_from_solark(self, solark_data):
        # Set AC wiring type based on Sol-Ark register 286 (Grid Type)
        sunspec_ac_type = 0  # Default to Unknown
        if solark_data.grid_type == 0x00:  # Single-phase
            sunspec_ac_type = 0  # Single Phase
        elif solark_data.grid_type == 0x01:  # Split-phase
            sunspec_ac_type = 1  # Split Phase
        elif solark_data.grid_type == 0x02:  # Three-phase Wye
            sunspec_ac_type = 2  # Three Phase Wye
        
        # SunSpec Operating State - Offset (2)
        self._set_register(SunSpecRegisterMap.GRID_AC_TYPE, sunspec_ac_type)
        
        # SunSpec Operating State - Offset (3)
        # Based on Sol-Ark inverter status
        self._set_register(SunSpecRegisterMap.GRID_OPERATING_STATE, 1 if solark_data.inverter_status == 2 else 0)
        
        # Inverter state mapping
        inv_state = 0  # Default to OFF
        if solark_data.inverter_status == 1:  # Self-test
            inv_state = 2  # STARTING
        elif solark_data.inverter_status == 2:  # Normal
            if solark_data.grid_power > 100:
                inv_state = 3  # RUNNING
            else:
                inv_state = 7  # STANDBY
        elif solark_data.inverter_status == 3:  # Alarm
            inv_state = 4  # THROTTLED
        elif solark_data.inverter_status == 4:  # Fault
            inv_state = 6  # FAULT
        else:
            inv_state = 0  # OFF
        
        # SunSpec Grid Connection State - Offset (4)
        self._set_register(SunSpecRegisterMap.GRID_STATUS, inv_state)
        
        # SunSpec Grid Connection State - Offset (5)
        grid_connected = 1 if solark_data.grid_relay_status == 1 else 0
        self._set_register(SunSpecRegisterMap.GRID_CONNECTION, grid_connected)
        
        # DER operational characteristics
        der_mode = 0
        if solark_data.grid_relay_status == 1:  # Connected to grid
            der_mode |= 0x0000  # Grid Following
        else:  # Disconnected from grid
            der_mode |= 0x0001  # Grid Forming
        
        # SunSpec Grid Connection State - Offset (8)
        # Options: GRID_FOLLOWING, GRID_FORMING, PV_CLIPPED
        self._set_register(SunSpecRegisterMap.GRID_DER_MODE, (der_mode >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.GRID_DER_MODE + 1, der_mode & 0xFFFF)
        
        # Alarm bitfield
        self._set_register(SunSpecRegisterMap.GRID_ALARM, 0)
        self._set_register(SunSpecRegisterMap.GRID_ALARM + 1, 0)
        
        # Power measurements - Grid CSV mapping uses register 169 (Total power of grid side L1L2)
        self._set_register(SunSpecRegisterMap.GRID_AC_POWER, int(solark_data.grid_power))
        
        # Apparent Power (VA) - Grid CSV maps to Sol-Ark register 38 "Apparent Power reading"
        self._set_register(SunSpecRegisterMap.GRID_AC_VA, int(solark_data.apparent_power))
        
        # Reactive Power (VAR) - Grid CSV calculated as sqrt(Register 38^2 - Register 169^2)
        try:
            # Calculate reactive power using the formula from CSV: sqrt(VA^2 - W^2)
            va_squared = solark_data.apparent_power ** 2
            w_squared = solark_data.grid_power ** 2
            if va_squared >= w_squared:
                reactive_power = int((va_squared - w_squared) ** 0.5)
                # Determine sign based on power factor (leading/lagging)
                if solark_data.grid_power_factor < 0:
                    reactive_power = -reactive_power
            else:
                reactive_power = 0  # Avoid negative square root
            self._set_register(SunSpecRegisterMap.GRID_AC_VAR, reactive_power)
        except (ValueError, ZeroDivisionError):
            self._set_register(SunSpecRegisterMap.GRID_AC_VAR, 0)
        
        # Power Factor (PF) - Grid CSV maps to Sol-Ark register 89 "Grid Real Power Factor"
        # Scale by 100 for SunSpec (PF_SF = -2, so 1.0 = 100)
        self._set_register(SunSpecRegisterMap.GRID_AC_PF, int(solark_data.grid_power_factor * 100))
        
        # Current and voltage measurements - Grid CSV mapping uses registers 160/161, 152, 150, 151
        grid_current_total = abs(solark_data.grid_current_l1 + solark_data.grid_current_l2)
        self._set_register(SunSpecRegisterMap.GRID_AC_CURRENT, int(grid_current_total * 100))  # Scale by 100
        self._set_register(SunSpecRegisterMap.GRID_AC_VOLTAGE_LL, int(solark_data.grid_voltage_l1l2 * 10))  # Register 152 - Line1-to-Line2
        self._set_register(SunSpecRegisterMap.GRID_AC_VOLTAGE_LN, int(solark_data.grid_voltage_l1n * 10))  # Register 150 - Line1-to-Neutral
        
        # Frequency - Grid CSV mapping uses register 79 (Grid frequency)
        frequency_scaled = int(solark_data.grid_frequency * 100)
        self._set_register(SunSpecRegisterMap.GRID_AC_FREQUENCY, (frequency_scaled >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.GRID_AC_FREQUENCY + 1, frequency_scaled & 0xFFFF)
        
        # Energy measurements - Grid CSV mapping uses registers 77 (Day Grid Sell), 76 (Day Grid Buy)
        energy_injected_wh = int(solark_data.grid_sell_energy * 1000)  # Convert kWh to Wh
        energy_absorbed_wh = int(solark_data.grid_buy_energy * 1000)   # Convert kWh to Wh
        
        # Total Energy Injected (TotWhInj) - 4 registers for uint64
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 19, 0)  # High 32 bits (upper 16)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 20, 0)  # High 32 bits (lower 16)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 21, (energy_injected_wh >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 22, energy_injected_wh & 0xFFFF)
        
        # Total Energy Absorbed (TotWhAbs) - 4 registers for uint64
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 23, 0)  # High 32 bits (upper 16)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 24, 0)  # High 32 bits (lower 16)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 25, (energy_absorbed_wh >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 26, energy_absorbed_wh & 0xFFFF)
        
        # Temperature measurements - shared with Load model
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 37, int(solark_data.igbt_temp * 10))  # Heat Sink temp
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 38, int(solark_data.dcdc_xfrmr_temp * 10))  # Transformer temp
        
        # Phase L1 measurements - Grid CSV mapping uses registers 167 (Grid side L1 power), 160 (Grid side current L1)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 41, int(solark_data.grid_power / 2))  # WL1: Approximate L1 power
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 45, int(solark_data.grid_current_l1 * 100))  # Current L1
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 46, int(solark_data.grid_voltage_l1l2 * 10))  # VL1L2: Grid voltage L1-L2
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 47, int(solark_data.grid_voltage_l1n * 10))  # VL1: Grid voltage L1-N
        
        # Phase L2 measurements - Grid CSV mapping uses registers 168 (Grid side L2 power), 161 (Grid side current L2)
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 64, int(solark_data.grid_power / 2))  # WL2: Approximate L2 power
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 68, int(solark_data.grid_current_l2 * 100))  # Current L2
        self._set_register(SunSpecRegisterMap.GRID_MODEL_BASE + 70, int(solark_data.grid_voltage_l2n * 10))  # VL2: Grid voltage L2-N
        
        # Vendor-specific status information
        alarm_info = f"Power:{solark_data.grid_power:.0f}W"
        self._set_string_registers(SunSpecRegisterMap.GRID_MODEL_BASE + 123, alarm_info, 32)
    
    
    ###############################################
    # Load Model (701) header - Second instance
    ###############################################
    def update_load_registers_from_solark(self, solark_data):
        # Set AC wiring type based on Sol-Ark register 286 (Grid Type) - same as Grid
        sunspec_ac_type = 0  # Default to Unknown
        if solark_data.grid_type == 0x00:  # Single-phase
            sunspec_ac_type = 0  # Single Phase
        elif solark_data.grid_type == 0x01:  # Split-phase
            sunspec_ac_type = 1  # Split Phase
        elif solark_data.grid_type == 0x02:  # Three-phase Wye
            sunspec_ac_type = 2  # Three Phase Wye
        
        self._set_register(SunSpecRegisterMap.LOAD_AC_TYPE, sunspec_ac_type)
        
        # Operating state based on Sol-Ark inverter status
        self._set_register(SunSpecRegisterMap.LOAD_OPERATING_STATE, 0xFFFF)
        
        self._set_register(SunSpecRegisterMap.LOAD_STATUS, 0XFFFF)
        
        # Grid connection state - same as Grid model
        self._set_register(SunSpecRegisterMap.LOAD_CONNECTION, 0xFFFF)
        
        # DER operational characteristics - same as Grid model
        der_mode = 0
        if solark_data.grid_relay_status == 1:  # Connected to grid
            der_mode |= 0x0000  # Grid Following
        else:  # Disconnected from grid
            der_mode |= 0x0001  # Grid Forming
        
        self._set_register(SunSpecRegisterMap.LOAD_DER_MODE, (der_mode >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.LOAD_DER_MODE + 1, der_mode & 0xFFFF)
        
        # Alarm bitfield
        self._set_register(SunSpecRegisterMap.LOAD_ALARM, 0)
        self._set_register(SunSpecRegisterMap.LOAD_ALARM + 1, 0)
        
        # Power measurements - Load CSV mapping uses register 178 (Load side total power)
        self._set_register(SunSpecRegisterMap.LOAD_AC_POWER, int(solark_data.load_power_total))
        
        # Apparent Power (VA) - Load CSV maps to Sol-Ark register 38 "Apparent Power reading"
        self._set_register(SunSpecRegisterMap.LOAD_AC_VA, int(solark_data.apparent_power))
        
        # Reactive Power (VAR) - Load CSV calculated as sqrt(Register 38^2 - Register 178^2)
        try:
            # Calculate reactive power using the formula from CSV: sqrt(VA^2 - W^2)
            va_squared = solark_data.apparent_power ** 2
            w_squared = solark_data.load_power_total ** 2
            if va_squared >= w_squared:
                reactive_power = int((va_squared - w_squared) ** 0.5)
                # For load side, assume lagging power factor (positive VAR)
                # since load typically consumes reactive power
            else:
                reactive_power = 0  # Avoid negative square root
            self._set_register(SunSpecRegisterMap.LOAD_AC_VAR, reactive_power)
        except (ValueError, ZeroDivisionError):
            self._set_register(SunSpecRegisterMap.LOAD_AC_VAR, 0)
        
        # Power Factor (PF) - Load CSV shows "UNIMPLEMENTED" for load side power factor
        self._set_register(SunSpecRegisterMap.LOAD_AC_PF, self.NULL_INT16)  # Not implemented per CSV
        
        # Current and voltage measurements - Load CSV mapping uses registers 179/180, 157/158, 157
        load_current_total = abs(solark_data.load_current_l1 + solark_data.load_current_l2)
        self._set_register(SunSpecRegisterMap.LOAD_AC_CURRENT, int(load_current_total * 100))  # Scale by 100
        # Using inverter voltage as approximation for load voltage (registers 157/158 not directly available)
        self._set_register(SunSpecRegisterMap.LOAD_AC_VOLTAGE_LL, int(solark_data.inverter_voltage * 10))  # Registers 157+158
        self._set_register(SunSpecRegisterMap.LOAD_AC_VOLTAGE_LN, int(solark_data.inverter_voltage_ln * 10))  # Register 157
        
        # Frequency - Load CSV mapping uses register 192 (Load frequency)
        frequency_scaled = int(solark_data.load_frequency * 100)
        self._set_register(SunSpecRegisterMap.LOAD_AC_FREQUENCY, (frequency_scaled >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.LOAD_AC_FREQUENCY + 1, frequency_scaled & 0xFFFF)
        
        # Energy measurements - Load CSV mapping uses registers 60 (Day Active Power Wh), 61 (Day Reactive Power Wh)
        energy_injected_wh = int(solark_data.load_energy * 1000)  # Convert kWh to Wh
        
        # Total Energy Injected (TotWhInj) - 4 registers for uint64
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 19, 0)  # High 32 bits (upper 16)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 20, 0)  # High 32 bits (lower 16)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 21, (energy_injected_wh >> 16) & 0xFFFF)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 22, energy_injected_wh & 0xFFFF)
        
        # Temperature measurements - shared with Grid model
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 37, int(solark_data.igbt_temp * 10))  # Heat Sink temp
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 38, int(solark_data.dcdc_xfrmr_temp * 10))  # Transformer temp
        
        # Phase L1 measurements - Load CSV mapping uses registers 176 (Load side L1 Power), 179 (Load current L1)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 41, int(solark_data.load_power_l1))  # WL1: Load L1 power
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 45, int(solark_data.load_current_l1 * 100))  # Current L1
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 46, int(solark_data.inverter_voltage * 10))  # VL1L2: Load voltage
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 47, int(solark_data.inverter_voltage_ln * 10))  # VL1: Load voltage L1
        
        # Phase L2 measurements - Load CSV mapping uses registers 177 (Load side L2 power), 180 (Load current L2)
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 64, int(solark_data.load_power_l2))  # WL2: Load L2 power
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 68, int(solark_data.load_current_l2 * 100))  # Current L2
        self._set_register(SunSpecRegisterMap.LOAD_MODEL_BASE + 70, int(solark_data.inverter_voltage_l2n * 10))  # VL2: Load voltage L2
        
        # Vendor-specific status information
        alarm_info = f"Load Model - Power:{solark_data.load_power_total:.0f}W L1:{solark_data.load_power_l1:.0f}W L2:{solark_data.load_power_l2:.0f}W"
        self._set_string_registers(SunSpecRegisterMap.LOAD_MODEL_BASE + 123, alarm_info, 32)
    
    
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
    
    def _update_dc_model_from_solark(self, solark_data):
        """Update DC model (714) registers with Sol-Ark data"""
        try:
            # Update DC model data from Sol-Ark registers
            # PV Port 1 - Sol-Ark registers 109 (DC voltage 1), 110 (DC current 1), 186 (PV1 input power)
            if hasattr(solark_data, 'pv1_voltage') and hasattr(solark_data, 'pv1_current') and hasattr(solark_data, 'pv1_power'):
                self.dc_model.ports[0].dc_voltage = solark_data.pv1_voltage
                self.dc_model.ports[0].dc_current = solark_data.pv1_current
                self.dc_model.ports[0].dc_power = solark_data.pv1_power
                self.dc_model.ports[0].dc_status = 1 if solark_data.pv1_power > 10 else 0  # ON if power > 10W
                self.dc_model.ports[0].temperature = solark_data.igbt_temp if hasattr(solark_data, 'igbt_temp') else 25.0
            
            # PV Port 2 - Sol-Ark registers 111 (DC voltage 2), 112 (DC current 2), 187 (PV2 input power)
            if hasattr(solark_data, 'pv2_voltage') and hasattr(solark_data, 'pv2_current') and hasattr(solark_data, 'pv2_power'):
                self.dc_model.ports[1].dc_voltage = solark_data.pv2_voltage
                self.dc_model.ports[1].dc_current = solark_data.pv2_current
                self.dc_model.ports[1].dc_power = solark_data.pv2_power
                self.dc_model.ports[1].dc_status = 1 if solark_data.pv2_power > 10 else 0  # ON if power > 10W
                self.dc_model.ports[1].temperature = solark_data.igbt_temp if hasattr(solark_data, 'igbt_temp') else 25.0
            
            # PV Port 3 - Sol-Ark registers 113 (DC voltage 3), 114 (DC current 3), 188 (PV3 input power)
            if hasattr(solark_data, 'pv3_voltage') and hasattr(solark_data, 'pv3_current') and hasattr(solark_data, 'pv3_power'):
                self.dc_model.ports[2].dc_voltage = solark_data.pv3_voltage
                self.dc_model.ports[2].dc_current = solark_data.pv3_current
                self.dc_model.ports[2].dc_power = solark_data.pv3_power
                self.dc_model.ports[2].dc_status = 1 if solark_data.pv3_power > 10 else 0  # ON if power > 10W
                self.dc_model.ports[2].temperature = solark_data.igbt_temp if hasattr(solark_data, 'igbt_temp') else 25.0
            
            # PV Port 4 - Keep uninitialized as requested
            # No updates for port 4 - it remains uninitialized
            
            # ESS Port 1 - Sol-Ark registers 183 (Battery voltage), 191 (Battery current), 190 (Battery power)
            self.dc_model.ports[4].dc_voltage = solark_data.battery_voltage
            self.dc_model.ports[4].dc_current = solark_data.battery_current
            self.dc_model.ports[4].dc_power = solark_data.battery_power
            self.dc_model.ports[4].dc_status = 1 if abs(solark_data.battery_power) > 10 else 0  # ON if power > 10W
            self.dc_model.ports[4].temperature = solark_data.battery_temperature
            
            # Calculate totals for all active ports (excluding port 4 which is uninitialized)
            total_current = 0.0
            total_power = 0.0
            
            for i, port in enumerate(self.dc_model.ports):
                if i != 3:  # Skip port 4 (uninitialized)
                    total_current += abs(port.dc_current)
                    total_power += port.dc_power
            
            self.dc_model.total_dc_current = total_current
            self.dc_model.total_dc_power = total_power
            
            # Update DC model registers
            self._update_dc_registers()
            
            self.logger.debug("Updated SunSpec DC model (714) with Sol-Ark data")
            
        except Exception as e:
            self.logger.error(f"Error updating DC model: {e}")
    
    def _update_dc_registers(self):
        """Update DC model (714) registers"""
        # Update total measurements
        self._set_register(SunSpecRegisterMap.DC_TOTAL_CURRENT, int(self.dc_model.total_dc_current * 100))  # Scale by 100
        self._set_register(SunSpecRegisterMap.DC_TOTAL_POWER, int(self.dc_model.total_dc_power))
        
        # Update port alarms (none for now)
        self._set_register_32bit(SunSpecRegisterMap.DC_PORT_ALARMS, 0)
        
        # Update individual port registers
        port_bases = [SunSpecRegisterMap.DC_PORT1_BASE, SunSpecRegisterMap.DC_PORT2_BASE,
                     SunSpecRegisterMap.DC_PORT3_BASE, SunSpecRegisterMap.DC_PORT4_BASE,
                     SunSpecRegisterMap.DC_PORT5_BASE]
        
        for i, port_base in enumerate(port_bases):
            port = self.dc_model.ports[i]
            
            # Skip port 4 (PV4) - keep it uninitialized
            if i == 3:
                continue
            
            # Update port measurements
            self._set_register(port_base + 10, int(port.dc_current * 100))  # DCA - scale by 100
            self._set_register(port_base + 11, int(port.dc_voltage * 10))   # DCV - scale by 10
            self._set_register(port_base + 12, int(port.dc_power))          # DCW
            self._set_register(port_base + 21, int(port.temperature * 10))  # Tmp - scale by 10
            self._set_register(port_base + 22, port.dc_status)              # DCSta
            
            # Update energy registers (simplified - could be enhanced with actual energy tracking)
            energy_injected = int(port.dc_energy_injected)
            energy_absorbed = int(port.dc_energy_absorbed)
            
            # DCWhInj (4 registers for uint64)
            self._set_register(port_base + 13, 0)  # High 32 bits (upper 16)
            self._set_register(port_base + 14, 0)  # High 32 bits (lower 16)
            self._set_register(port_base + 15, (energy_injected >> 16) & 0xFFFF)  # Low 32 bits (upper 16)
            self._set_register(port_base + 16, energy_injected & 0xFFFF)  # Low 32 bits (lower 16)
            
            # DCWhAbs (4 registers for uint64)
            self._set_register(port_base + 17, 0)  # High 32 bits (upper 16)
            self._set_register(port_base + 18, 0)  # High 32 bits (lower 16)
            self._set_register(port_base + 19, (energy_absorbed >> 16) & 0xFFFF)  # Low 32 bits (upper 16)
            self._set_register(port_base + 20, energy_absorbed & 0xFFFF)  # Low 32 bits (lower 16)
            
            # Update alarm register (32-bit) - no alarms for now
            self._set_register_32bit(port_base + 23, port.dc_alarm)
    
    def get_register_value(self, address):
        """Get register value by address"""
        return self.registers.get(address)
    
    def get_all_registers(self):
        """Get all register values"""
        return self.registers.copy()