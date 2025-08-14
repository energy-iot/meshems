"""
Inverter Factory Implementation

This module provides a factory pattern for creating inverter clients
based on the inverter type specified in the configuration.

All inverter types now use JSON-based register mapping for consistency and extensibility.
"""

from typing import Optional
from .base import InverterClient
from .generic_mapping import GenericRegisterMapping
from .generic_inverter import GenericInverterClient


class InverterFactory:
    """Factory for creating inverter clients - JSON mapping only"""
    
    @staticmethod
    def create_inverter(inverter_type: str, port: str, baudrate: int, 
                       modbus_address: int, **kwargs) -> Optional[InverterClient]:
        """
        Create an inverter client based on the inverter type
        
        All inverter types now use JSON-based register mapping for consistency.
        
        Args:
            inverter_type: Type of inverter ("solark", "generic", etc.)
            port: Serial port path
            baudrate: Serial baudrate
            modbus_address: Modbus slave address
            **kwargs: Additional arguments specific to inverter types
                - config_file: JSON register mapping file (required)
            
        Returns:
            InverterClient instance or None if type not supported
        """
        config_file = kwargs.get("config_file")
        if not config_file:
            # Default config files based on inverter type
            if inverter_type == "solark":
                config_file = "solark_registers.json"
            elif inverter_type == "generic":
                config_file = "generic_registers.json"
            else:
                return None
        
        if inverter_type == "solark":
            # Import here to avoid circular imports
            from .solark_implementation import SolArkModbusClient
            
            return SolArkModbusClient(
                port, baudrate, modbus_address,
                config_file=config_file
            )
        elif inverter_type == "generic":
            try:
                register_mapping = GenericRegisterMapping(config_file)
                return GenericInverterClient(port, baudrate, modbus_address, register_mapping)
            except Exception:
                # If we can't load the config file, return None
                return None
        # Add other inverter types as needed - all must use JSON mapping
        else:
            return None
    
    @staticmethod
    def get_supported_types() -> list:
        """Get list of supported inverter types"""
        return ["solark", "generic"]
    
    @staticmethod
    def get_default_config_file(inverter_type: str) -> Optional[str]:
        """Get the default config file for an inverter type"""
        defaults = {
            "solark": "solark_registers.json",
            "generic": "generic_registers.json"
        }
        return defaults.get(inverter_type)