"""
Sol-Ark Register Mapping and Scaling Factors

This module defines the register mappings and scaling factors for Sol-Ark inverters,
ported from the C++ implementation.
"""

from enum import Enum
from dataclasses import dataclass
from typing import List


class SolArkRegisterMap:
    """Sol-Ark Modbus register addresses"""
    
    # Diagnostic registers
    COMM_VERSION = 2
    SN_BYTE_01 = 3
    SN_BYTE_02 = 4
    SN_BYTE_03 = 5
    SN_BYTE_04 = 6
    SN_BYTE_05 = 7
    
    # Energy registers
    BATTERY_CHARGE_ENERGY = 70
    BATTERY_DISCHARGE_ENERGY = 71
    GRID_BUY_ENERGY = 76
    GRID_SELL_ENERGY = 77
    GRID_FREQUENCY = 79
    LOAD_ENERGY = 84
    PV_ENERGY = 108
    
    # Grid and inverter registers
    GRID_VOLTAGE = 152
    INVERTER_VOLTAGE = 156
    GRID_CURRENT_L1 = 160
    GRID_CURRENT_L2 = 161
    GRID_CT_CURRENT_L1 = 162
    GRID_CT_CURRENT_L2 = 163
    INVERTER_CURRENT_L1 = 164
    INVERTER_CURRENT_L2 = 165
    SMART_LOAD_POWER = 166
    GRID_POWER = 169
    
    # Inverter status register
    INVERTER_STATUS = 59  # 1=Self-test, 2=Normal, 3=Alarm, 4=Fault
    
    # Temperature registers
    DCDC_XFRMR_TEMP = 90
    IGBT_HEATSINK_TEMP = 91
    
    # Power and battery registers
    INVERTER_OUTPUT_POWER = 175
    LOAD_POWER_L1 = 176
    LOAD_POWER_L2 = 177
    LOAD_POWER_TOTAL = 178
    LOAD_CURRENT_L1 = 179
    LOAD_CURRENT_L2 = 180
    BATTERY_TEMPERATURE = 182
    BATTERY_VOLTAGE = 183
    BATTERY_SOC = 184
    PV1_POWER = 186
    PV2_POWER = 187
    
    # Battery status registers
    BATTERY_POWER = 190
    BATTERY_CURRENT = 191
    LOAD_FREQUENCY = 192
    INVERTER_FREQUENCY = 193
    GRID_RELAY_STATUS = 194
    GENERATOR_RELAY_STATUS = 195
    
    # Battery configuration registers
    BATTERY_CAPACITY = 204
    CORRECTED_BATTERY_CAPACITY = 107
    BATTERY_EMPTY_VOLTAGE = 205
    BATTERY_SHUTDOWN_VOLTAGE = 220
    BATTERY_RESTART_VOLTAGE = 221
    BATTERY_LOW_VOLTAGE = 222
    BATTERY_SHUTDOWN_PERCENT = 217
    BATTERY_RESTART_PERCENT = 218
    BATTERY_LOW_PERCENT = 219
    
    # BMS registers (for lithium batteries)
    BMS_CHARGING_VOLTAGE = 312
    BMS_DISCHARGE_VOLTAGE = 313
    BMS_CHARGING_CURRENT_LIMIT = 314
    BMS_DISCHARGE_CURRENT_LIMIT = 315
    BMS_REAL_TIME_SOC = 316
    BMS_REAL_TIME_VOLTAGE = 317
    BMS_REAL_TIME_CURRENT = 318
    BMS_REAL_TIME_TEMP = 319
    BMS_WARNING = 322
    BMS_FAULT = 323
    GRID_TYPE = 286  # 0=Single, 1=Split, 2=Three-phase


class SolArkScalingFactors:
    """Scaling factors for Sol-Ark register values"""
    
    VOLTAGE = 10.0
    CURRENT = 100.0
    ENERGY = 10.0
    FREQUENCY = 100.0
    TEMPERATURE_OFFSET = 1000.0
    TEMPERATURE_SCALE = 10.0


class SolArkBlockType(Enum):
    """Enum to identify different logical blocks of Sol-Ark registers"""
    
    ENERGY = "energy"
    PV_ENERGY = "pv_energy"
    INVERTER_STATUS = "inverter_status"
    TEMPERATURES = "temperatures"
    GRID_INVERTER_150 = "grid_inverter_150"
    POWER_BATTERY_170 = "power_battery_170"
    BATTERY_STATUS_190 = "battery_status_190"
    BATTERY_CAPACITY_204 = "battery_capacity_204"
    CORRECTED_BATTERY_CAPACITY_107 = "corrected_battery_capacity_107"
    BATTERY_EMPTY_VOLTAGE_205 = "battery_empty_voltage_205"
    BATTERY_VOLTAGE_THRESHOLDS_220 = "battery_voltage_thresholds_220"
    BATTERY_PERCENT_THRESHOLDS_217 = "battery_percent_thresholds_217"
    BMS_DATA_312 = "bms_data_312"
    GRID_TYPE_286 = "grid_type_286"
    DIAGNOSTICS = "diagnostics"


@dataclass
class ModbusReadBlock:
    """Structure to define a block of Modbus registers to read"""
    
    block_type: SolArkBlockType
    start_register: int
    num_registers: int
    description: str


# Define the blocks of registers to be read (max 20 registers per block)
SOLARK_READ_BLOCKS: List[ModbusReadBlock] = [
    ModbusReadBlock(
        SolArkBlockType.ENERGY,
        SolArkRegisterMap.BATTERY_CHARGE_ENERGY,
        15,
        "Energy Data (70-84)"
    ),
    ModbusReadBlock(
        SolArkBlockType.PV_ENERGY,
        SolArkRegisterMap.PV_ENERGY,
        1,
        "PV Energy (108)"
    ),
    ModbusReadBlock(
        SolArkBlockType.INVERTER_STATUS,
        SolArkRegisterMap.INVERTER_STATUS,
        1,
        "Inverter Status (59)"
    ),
    ModbusReadBlock(
        SolArkBlockType.TEMPERATURES,
        SolArkRegisterMap.DCDC_XFRMR_TEMP,
        2,
        "Temperatures (90-91)"
    ),
    ModbusReadBlock(
        SolArkBlockType.GRID_INVERTER_150,
        150,
        20,
        "Grid/Inverter Data (150-169)"
    ),
    ModbusReadBlock(
        SolArkBlockType.POWER_BATTERY_170,
        170,
        20,
        "Power/Battery Data (170-189)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BATTERY_STATUS_190,
        SolArkRegisterMap.BATTERY_POWER,
        10,
        "Battery Status (190-199)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BATTERY_CAPACITY_204,
        SolArkRegisterMap.BATTERY_CAPACITY,
        1,
        "Battery Capacity (204)"
    ),
    ModbusReadBlock(
        SolArkBlockType.CORRECTED_BATTERY_CAPACITY_107,
        SolArkRegisterMap.CORRECTED_BATTERY_CAPACITY,
        1,
        "Corrected Battery Capacity (107)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BATTERY_EMPTY_VOLTAGE_205,
        SolArkRegisterMap.BATTERY_EMPTY_VOLTAGE,
        1,
        "Battery Empty Voltage (205)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BATTERY_VOLTAGE_THRESHOLDS_220,
        SolArkRegisterMap.BATTERY_SHUTDOWN_VOLTAGE,
        3,
        "Battery Voltage Thresholds (220-222)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BATTERY_PERCENT_THRESHOLDS_217,
        SolArkRegisterMap.BATTERY_SHUTDOWN_PERCENT,
        3,
        "Battery Percent Thresholds (217-219)"
    ),
    ModbusReadBlock(
        SolArkBlockType.BMS_DATA_312,
        SolArkRegisterMap.BMS_CHARGING_VOLTAGE,
        12,
        "BMS Data (312-323)"
    ),
    ModbusReadBlock(
        SolArkBlockType.GRID_TYPE_286,
        SolArkRegisterMap.GRID_TYPE,
        1,
        "Grid Type (286)"
    ),
    ModbusReadBlock(
        SolArkBlockType.DIAGNOSTICS,
        SolArkRegisterMap.COMM_VERSION,
        6,
        "Diagnostics (2-7)"
    ),
]