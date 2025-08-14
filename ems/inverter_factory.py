"""
Inverter Factory Implementation

This module provides a factory pattern for creating inverter clients
based on the inverter type specified in the configuration.
"""

from typing import Optional
from .base import InverterClient
from .generic_mapping import GenericRegisterMapping
from .generic_inverter import GenericInverterClient


class InverterFactory:
    """Factory for creating inverter clients"""
    
    @staticmethod
    def create_inverter(inverter_type: str, port: str, baudrate: int, 
                       modbus_address: int, **kwargs) -> Optional[InverterClient]:
        """
        Create an inverter client based on the inverter type
        
        Args:
            inverter_type: Type of inverter ("solark", "generic", etc.)
            port: Serial port path
            baudrate: Serial baudrate
            modbus_address: Modbus slave address
            **kwargs: Additional arguments specific to inverter types
            
        Returns:
            InverterClient instance or None if type not supported
        """
        if inverter_type == "solark":
            # Import here to avoid circular imports
            from .solark_implementation import SolArkModbusClient
            
            # Check for Sol-Ark specific configuration
            use_json_mapping = kwargs.get("use_json_mapping", False)
            config_file = kwargs.get("config_file", "solark_registers.json")
            
            return SolArkModbusClient(
                port, baudrate, modbus_address,
                use_json_mapping=use_json_mapping,
                config_file=config_file
            )
        elif inverter_type == "generic":
            config_file = kwargs.get("config_file", "generic_registers.json")
            try:
                register_mapping = GenericRegisterMapping(config_file)
                return GenericInverterClient(port, baudrate, modbus_address, register_mapping)
            except Exception:
                # If we can't load the config file, return None
                return None
        # Add other inverter types as needed
        else:
            return None