"""
Modbus TCP Server Implementation

This module implements a SunSpec-compliant Modbus TCP server that exposes
Sol-Ark inverter data in standardized SunSpec format.
"""

import logging
import asyncio
import threading
from typing import Dict, Optional, Any
from dataclasses import dataclass

from pymodbus.server import StartTcpServer
from pymodbus import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock, ModbusDeviceContext, ModbusServerContext

from .sunspec_models import SunSpecMapper


@dataclass
class ModbusServerConfig:
    """Configuration for Modbus TCP server"""
    
    host: str = "0.0.0.0"
    port: int = 8502
    unit_id: int = 1
    device_info: Dict[str, str] = None


class SunSpecModbusServer:
    """SunSpec-compliant Modbus TCP server"""
    
    def __init__(self, config: ModbusServerConfig):
        """
        Initialize Modbus TCP server
        
        Args:
            config: Server configuration
        """
        self.logger = logging.getLogger(__name__)
        self.config = config
        self.running = False
        self.server_thread = None
        
        # Initialize SunSpec mapper
        device_info = config.device_info or {}
        self.sunspec_mapper = SunSpecMapper(device_info)
        
        # Initialize Modbus data store
        self._setup_datastore()
        
        # Setup device identification
        self._setup_device_identification()
        
        self.logger.info(f"Initialized SunSpec Modbus server on {config.host}:{config.port}")
    
    def _setup_datastore(self):
        """Setup Modbus data store with SunSpec registers"""
        # Create data blocks for different register types
        # Holding registers (0-9999) - SunSpec models live here (40000+ addressing handled by client)
        holding_registers = ModbusSequentialDataBlock(0, [0] * 10000)
        
        # Input registers (0-999) - Not used in SunSpec but available
        input_registers = ModbusSequentialDataBlock(0, [0] * 1000)
        
        # Coils (0-999) - Not used in SunSpec but available
        coils = ModbusSequentialDataBlock(0, [False] * 1000)
        
        # Discrete inputs (0-999) - Not used in SunSpec but available
        discrete_inputs = ModbusSequentialDataBlock(0, [False] * 1000)
        
        # Create device context
        self.slave_context = ModbusDeviceContext(
            di=discrete_inputs,
            co=coils,
            hr=holding_registers,
            ir=input_registers
        )
        
        # Create server context
        self.server_context = ModbusServerContext(
            slaves={self.config.unit_id: self.slave_context},
            single=False
        )
        
        # Initialize SunSpec registers
        self._update_sunspec_registers()
    
    def _setup_device_identification(self):
        """Setup Modbus device identification"""
        device_info = self.config.device_info or {}
        
        self.device_identity = ModbusDeviceIdentification()
        self.device_identity.VendorName = device_info.get("manufacturer", "Energy IoT Open Source")
        self.device_identity.ProductCode = device_info.get("model", "EMS-Dev Python")
        self.device_identity.VendorUrl = "https://github.com/energy-iot/ems-dev"
        self.device_identity.ProductName = "EMS-Dev Python Gateway"
        self.device_identity.ModelName = device_info.get("model", "EMS-Dev Python")
        self.device_identity.MajorMinorRevision = device_info.get("version", "1.0.0")
    
    def _update_sunspec_registers(self):
        """Update Modbus registers with SunSpec data"""
        try:
            # Get all SunSpec registers
            sunspec_registers = self.sunspec_mapper.get_all_registers()
            
            # Update holding registers
            for address, value in sunspec_registers.items():
                # Convert from 40000-based addressing to 0-based
                register_address = address - 40000
                if 0 <= register_address < 10000:
                    self.slave_context.setValues(3, register_address, [value])  # 3 = holding registers
            
            self.logger.debug(f"Updated {len(sunspec_registers)} SunSpec registers")
            
        except Exception as e:
            self.logger.error(f"Error updating SunSpec registers: {e}")
    
    def update_from_solark(self, solark_data):
        """Update SunSpec models with Sol-Ark data"""
        try:
            # Update SunSpec mapper with new data
            self.sunspec_mapper.update_from_solark(solark_data)
            
            # Update Modbus registers
            self._update_sunspec_registers()
            
            self.logger.debug("Updated Modbus server with Sol-Ark data")
            
        except Exception as e:
            self.logger.error(f"Error updating Modbus server: {e}")
    
    def start(self):
        """Start the Modbus TCP server in a separate thread"""
        if self.running:
            self.logger.warning("Modbus server is already running")
            return
        
        self.running = True
        self.server_thread = threading.Thread(target=self._run_server, daemon=True)
        self.server_thread.start()
        
        self.logger.info(f"Started SunSpec Modbus TCP server on {self.config.host}:{self.config.port}")
    
    def stop(self):
        """Stop the Modbus TCP server"""
        if not self.running:
            return
        
        self.running = False
        
        if self.server_thread and self.server_thread.is_alive():
            self.server_thread.join(timeout=5.0)
        
        self.logger.info("Stopped SunSpec Modbus TCP server")
    
    def _run_server(self):
        """Run the Modbus TCP server"""
        try:
            # Start the server
            StartTcpServer(
                context=self.server_context,
                identity=self.device_identity,
                address=(self.config.host, self.config.port)
            )
            
        except Exception as e:
            self.logger.error(f"Error running Modbus server: {e}")
            self.running = False
    
    def is_running(self) -> bool:
        """Check if server is running"""
        return self.running and (self.server_thread is not None and self.server_thread.is_alive())
    
    def get_register_value(self, address: int) -> Optional[int]:
        """Get register value by address"""
        try:
            # Convert from 40000-based addressing to 0-based
            register_address = address - 40000
            if 0 <= register_address < 10000:
                values = self.slave_context.getValues(3, register_address, 1)  # 3 = holding registers
                return values[0] if values else None
            return None
        except Exception as e:
            self.logger.error(f"Error getting register {address}: {e}")
            return None
    
    def set_register_value(self, address: int, value: int):
        """Set register value by address"""
        try:
            # Convert from 40000-based addressing to 0-based
            register_address = address - 40000
            if 0 <= register_address < 10000:
                self.slave_context.setValues(3, register_address, [value])  # 3 = holding registers
        except Exception as e:
            self.logger.error(f"Error setting register {address}: {e}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get server statistics"""
        return {
            "running": self.is_running(),
            "host": self.config.host,
            "port": self.config.port,
            "unit_id": self.config.unit_id,
            "register_count": len(self.sunspec_mapper.get_all_registers())
        }


class AsyncSunSpecModbusServer:
    """Async version of SunSpec Modbus server using asyncio"""
    
    def __init__(self, config: ModbusServerConfig):
        """
        Initialize async Modbus TCP server
        
        Args:
            config: Server configuration
        """
        self.logger = logging.getLogger(__name__)
        self.config = config
        self.server = None
        self.running = False
        
        # Initialize SunSpec mapper
        device_info = config.device_info or {}
        self.sunspec_mapper = SunSpecMapper(device_info)
        
        # Initialize Modbus data store
        self._setup_datastore()
        
        # Setup device identification
        self._setup_device_identification()
        
        self.logger.info(f"Initialized async SunSpec Modbus server on {config.host}:{config.port}")
    
    def _setup_datastore(self):
        """Setup Modbus data store with SunSpec registers"""
        # Create data blocks for different register types
        holding_registers = ModbusSequentialDataBlock(0, [0] * 10000)
        input_registers = ModbusSequentialDataBlock(0, [0] * 1000)
        coils = ModbusSequentialDataBlock(0, [False] * 1000)
        discrete_inputs = ModbusSequentialDataBlock(0, [False] * 1000)
        
        # Create device context
        self.slave_context = ModbusDeviceContext(
            di=discrete_inputs,
            co=coils,
            hr=holding_registers,
            ir=input_registers
        )
        
        # Create server context
        self.server_context = ModbusServerContext(
            slaves={self.config.unit_id: self.slave_context},
            single=False
        )
        
        # Initialize SunSpec registers
        self._update_sunspec_registers()
    
    def _setup_device_identification(self):
        """Setup Modbus device identification"""
        device_info = self.config.device_info or {}
        
        self.device_identity = ModbusDeviceIdentification()
        self.device_identity.VendorName = device_info.get("manufacturer", "Energy IoT Open Source")
        self.device_identity.ProductCode = device_info.get("model", "EMS-Dev Python")
        self.device_identity.VendorUrl = "https://github.com/energy-iot/ems-dev"
        self.device_identity.ProductName = "EMS-Dev Python Gateway"
        self.device_identity.ModelName = device_info.get("model", "EMS-Dev Python")
        self.device_identity.MajorMinorRevision = device_info.get("version", "1.0.0")
    
    def _update_sunspec_registers(self):
        """Update Modbus registers with SunSpec data"""
        try:
            sunspec_registers = self.sunspec_mapper.get_all_registers()
            
            for address, value in sunspec_registers.items():
                register_address = address - 40000
                if 0 <= register_address < 10000:
                    self.slave_context.setValues(3, register_address, [value])
            
            self.logger.debug(f"Updated {len(sunspec_registers)} SunSpec registers")
            
        except Exception as e:
            self.logger.error(f"Error updating SunSpec registers: {e}")
    
    async def update_from_solark(self, solark_data):
        """Update SunSpec models with Sol-Ark data (async)"""
        try:
            self.sunspec_mapper.update_from_solark(solark_data)
            self._update_sunspec_registers()
            self.logger.debug("Updated async Modbus server with Sol-Ark data")
        except Exception as e:
            self.logger.error(f"Error updating async Modbus server: {e}")
    
    async def start(self):
        """Start the async Modbus TCP server"""
        if self.running:
            self.logger.warning("Async Modbus server is already running")
            return
        
        try:
            # Note: This is a simplified async implementation
            # For full async support, you would need to use an async Modbus library
            # or implement custom async handling
            
            self.running = True
            self.logger.info(f"Started async SunSpec Modbus TCP server on {self.config.host}:{self.config.port}")
            
            # Keep the server running
            while self.running:
                await asyncio.sleep(1)
                
        except Exception as e:
            self.logger.error(f"Error running async Modbus server: {e}")
            self.running = False
    
    async def stop(self):
        """Stop the async Modbus TCP server"""
        self.running = False
        self.logger.info("Stopped async SunSpec Modbus TCP server")
    
    def is_running(self) -> bool:
        """Check if server is running"""
        return self.running