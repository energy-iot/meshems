"""
Energy Management System (EMS) Python Implementation

A Python implementation of the EMS for Linux systems using /dev/tty RS485 ports.
Provides Modbus RTU communication with inverters and SunSpec-compliant TCP server.

All inverter types now use JSON-based register mapping for consistency and extensibility.
"""

__version__ = "1.0.0"
__author__ = "Energy IoT Open Source"

# Export main components of the new architecture
from .base import InverterClient, InverterData, RegisterMapping
from .inverter_factory import InverterFactory
from .generic_mapping import GenericRegisterMapping
from .generic_inverter import GenericInverterClient, GenericInverterData
# from .solark_implementation import SolArkModbusClient, SolArkData  # Module not found, commented out
from .sunspec_models import SunSpecMapper, SunSpecCommonModel, SunSpecGridModel, SunSpecLoadModel, SunSpecBatteryModel
from .modbus_server import SunSpecModbusServer, ModbusServerConfig
from .main import EMSApplication

__all__ = [
    # Base classes
    'InverterClient',
    'InverterData', 
    'RegisterMapping',
    
    # Factory and mapping
    'InverterFactory',
    'GenericRegisterMapping',
    
    # Inverter implementations
    'GenericInverterClient',
    'GenericInverterData',
    # 'SolArkModbusClient',  # Module not found, commented out
    # 'SolArkData',          # Module not found, commented out
    
    # SunSpec models
    'SunSpecMapper',
    'SunSpecCommonModel',
    'SunSpecGridModel',
    'SunSpecLoadModel', 
    'SunSpecBatteryModel',
    
    # Modbus server
    'SunSpecModbusServer',
    'ModbusServerConfig',
    
    # Main application
    'EMSApplication'
]