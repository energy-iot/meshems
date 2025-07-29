"""
EMS Main Application (Simple Version)

Main entry point for the Energy Management System Python implementation.
Uses only standard Python libraries to avoid import dependencies.
"""

import argparse
import json
import logging
import signal
import sys
import time
from pathlib import Path
from typing import Dict, Any, Optional

try:
    import yaml
except ImportError:
    yaml = None

from .solark_client import SolArkModbusClient, SolArkData
from .modbus_server import SunSpecModbusServer, ModbusServerConfig


class EMSApplication:
    """Main EMS application class"""
    
    def __init__(self, config_path: str = "config.yaml"):
        """
        Initialize EMS application
        
        Args:
            config_path: Path to configuration file
        """
        self.config_path = config_path
        self.config: Dict[str, Any] = {}
        self.running = False
        
        # Components
        self.solark_client: Optional[SolArkModbusClient] = None
        self.modbus_server: Optional[SunSpecModbusServer] = None
        
        # Load configuration
        self._load_config()
        
        # Setup logging
        self._setup_logging()
        
        self.logger = logging.getLogger(__name__)
        self.logger.info("EMS Application initialized")
    
    def _load_config(self):
        """Load configuration from YAML or JSON file"""
        try:
            config_file = Path(self.config_path)
            if config_file.exists():
                with open(config_file, 'r') as f:
                    if yaml and self.config_path.endswith('.yaml'):
                        self.config = yaml.safe_load(f)
                    else:
                        # Try JSON format as fallback
                        self.config = json.load(f)
                print(f"Loaded configuration from {self.config_path}")
            else:
                print(f"Configuration file {self.config_path} not found, using defaults")
                self.config = self._get_default_config()
        except Exception as e:
            print(f"Error loading configuration: {e}")
            self.config = self._get_default_config()
    
    def _get_default_config(self) -> Dict[str, Any]:
        """Get default configuration"""
        return {
            "serial": {
                "port": "/dev/ttyUSB0",
                "baudrate": 9600,
                "timeout": 1.0
            },
            "solark": {
                "modbus_address": 1,
                "poll_interval": 5.0,
                "max_retries": 3,
                "retry_delay": 0.5
            },
            "sunspec_server": {
                "enabled": True,
                "host": "0.0.0.0",
                "port": 8502,
                "unit_id": 1
            },
            "logging": {
                "level": "INFO",
                "format": "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
            },
            "device_info": {
                "manufacturer": "Energy IoT Open Source",
                "model": "EMS-Dev Python",
                "version": "1.0.0",
                "serial_number": "EMS-PY-001",
                "options": "Sol-Ark Gateway"
            },
            "monitoring": {
                "console_output": True,
                "console_update_interval": 10.0
            }
        }
    
    def _setup_logging(self):
        """Setup logging configuration"""
        log_config = self.config.get("logging", {})
        
        # Configure root logger
        logging.basicConfig(
            level=getattr(logging, log_config.get("level", "INFO")),
            format=log_config.get("format", "%(asctime)s - %(name)s - %(levelname)s - %(message)s"),
            handlers=[
                logging.StreamHandler(sys.stdout),
                logging.FileHandler(log_config.get("file", "ems.log"))
            ]
        )
    
    def _setup_signal_handlers(self):
        """Setup signal handlers for graceful shutdown"""
        def signal_handler(signum, frame):
            self.logger.info(f"Received signal {signum}, shutting down...")
            self.stop()
        
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)
    
    def _initialize_components(self):
        """Initialize all components"""
        try:
            # Initialize Sol-Ark client
            serial_config = self.config.get("serial", {})
            solark_config = self.config.get("solark", {})
            
            self.solark_client = SolArkModbusClient(
                port=serial_config.get("port", "/dev/ttyUSB0"),
                baudrate=serial_config.get("baudrate", 9600),
                modbus_address=solark_config.get("modbus_address", 1)
            )
            
            # Connect to Sol-Ark
            if not self.solark_client.connect():
                raise Exception("Failed to connect to Sol-Ark inverter")
            
            # Initialize Modbus server if enabled
            server_config = self.config.get("sunspec_server", {})
            if server_config.get("enabled", True):
                modbus_config = ModbusServerConfig(
                    host=server_config.get("host", "0.0.0.0"),
                    port=server_config.get("port", 8502),
                    unit_id=server_config.get("unit_id", 1),
                    device_info=self.config.get("device_info", {})
                )
                
                self.modbus_server = SunSpecModbusServer(modbus_config)
                self.modbus_server.start()
            
            self.logger.info("All components initialized successfully")
            
        except Exception as e:
            self.logger.error(f"Error initializing components: {e}")
            raise
    
    def _print_status_display(self, solark_data: SolArkData):
        """Print simple status display using standard print"""
        print("\n" + "="*80)
        print("EMS-DEV PYTHON GATEWAY - SOL-ARK MONITOR")
        print("="*80)
        
        # Battery status
        print("\nBATTERY STATUS:")
        print(f"  Power:       {solark_data.battery_power:8.1f} W")
        print(f"  Current:     {solark_data.battery_current:8.2f} A")
        print(f"  Voltage:     {solark_data.battery_voltage:8.2f} V")
        print(f"  SOC:         {solark_data.battery_soc:8.0f} %")
        print(f"  Temperature: {solark_data.battery_temperature:8.1f} °C")
        print(f"  Capacity:    {solark_data.battery_capacity:8.1f} Ah")
        
        # Status indicators
        status = "IDLE"
        if solark_data.battery_power < -50:
            status = "CHARGING"
        elif solark_data.battery_power > 50:
            status = "DISCHARGING"
        print(f"  Status:      {status}")
        
        # Grid/Power status
        print("\nPOWER & GRID:")
        print(f"  Grid Power:  {solark_data.grid_power:8.1f} W")
        print(f"  Grid Voltage:{solark_data.grid_voltage:8.1f} V")
        print(f"  Grid Freq:   {solark_data.grid_frequency:8.2f} Hz")
        print(f"  Load Power:  {solark_data.load_power_total:8.1f} W")
        print(f"  PV1 Power:   {solark_data.pv1_power:8.1f} W")
        print(f"  PV2 Power:   {solark_data.pv2_power:8.1f} W")
        print(f"  PV Total:    {solark_data.pv_power_total:8.3f} kW")
        
        # Grid status
        grid_status = "DISCONNECTED"
        if solark_data.grid_relay_status > 0:
            if solark_data.grid_power < -50:
                grid_status = "SELLING"
            elif solark_data.grid_power > 50:
                grid_status = "BUYING"
            else:
                grid_status = "CONNECTED"
        print(f"  Grid Status: {grid_status}")
        
        # Footer with timestamps
        print(f"\nLast Update: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(solark_data.last_update))}")
        if self.modbus_server and self.modbus_server.is_running():
            print(f"SunSpec Server: RUNNING on port {self.modbus_server.config.port}")
        else:
            print("SunSpec Server: STOPPED")
        
        print("="*80)
    
    def _poll_solark(self) -> bool:
        """Poll Sol-Ark inverter data"""
        try:
            if self.solark_client and self.solark_client.poll():
                # Update Modbus server if running
                if self.modbus_server:
                    self.modbus_server.update_from_solark(self.solark_client.data)
                
                return True
            else:
                self.logger.warning("Failed to poll Sol-Ark data")
                return False
                
        except Exception as e:
            self.logger.error(f"Error polling Sol-Ark: {e}")
            return False
    
    def run(self):
        """Run the main application loop"""
        try:
            self.logger.info("Starting EMS application...")
            
            # Setup signal handlers
            self._setup_signal_handlers()
            
            # Initialize components
            self._initialize_components()
            
            self.running = True
            
            # Get configuration
            poll_interval = self.config.get("solark", {}).get("poll_interval", 5.0)
            console_output = self.config.get("monitoring", {}).get("console_output", True)
            console_update_interval = self.config.get("monitoring", {}).get("console_update_interval", 10.0)
            
            last_console_update = 0
            
            if console_output:
                print("EMS-Dev Python Gateway Started")
                print(f"Polling Sol-Ark every {poll_interval} seconds")
                if self.modbus_server:
                    print(f"SunSpec server running on port {self.modbus_server.config.port}")
            
            # Main loop
            while self.running:
                start_time = time.time()
                
                # Poll Sol-Ark data
                poll_success = self._poll_solark()
                
                # Update console display
                if console_output and (time.time() - last_console_update) >= console_update_interval:
                    if poll_success and self.solark_client:
                        self._print_status_display(self.solark_client.data)
                    last_console_update = time.time()
                
                # Sleep for remaining poll interval
                elapsed = time.time() - start_time
                sleep_time = max(0, poll_interval - elapsed)
                if sleep_time > 0:
                    time.sleep(sleep_time)
            
        except KeyboardInterrupt:
            self.logger.info("Application interrupted by user")
        except Exception as e:
            self.logger.error(f"Application error: {e}")
            raise
        finally:
            self.stop()
    
    def stop(self):
        """Stop the application"""
        if not self.running:
            return
        
        self.logger.info("Stopping EMS application...")
        self.running = False
        
        # Stop Modbus server
        if self.modbus_server:
            self.modbus_server.stop()
        
        # Disconnect Sol-Ark client
        if self.solark_client:
            self.solark_client.disconnect()
        
        self.logger.info("EMS application stopped")


def main():
    """
    EMS-Dev Python Gateway
    
    Energy Management System for Sol-Ark inverters with SunSpec Modbus TCP server.
    """
    parser = argparse.ArgumentParser(
        description="EMS-Dev Python Gateway - Energy Management System for Sol-Ark inverters"
    )
    parser.add_argument('--config', '-c', default='config.yaml', 
                       help='Configuration file path (default: config.yaml)')
    parser.add_argument('--verbose', '-v', action='store_true', 
                       help='Enable verbose logging')
    parser.add_argument('--test', '-t', action='store_true', 
                       help='Test mode - single poll and exit')
    
    args = parser.parse_args()
    
    try:
        # Create application
        app = EMSApplication(args.config)
        
        # Override logging level if verbose
        if args.verbose:
            logging.getLogger().setLevel(logging.DEBUG)
        
        if args.test:
            # Test mode - single poll
            print("Running in test mode...")
            app._initialize_components()
            
            if app._poll_solark():
                print("✓ Sol-Ark poll successful")
                
                # Display data
                if app.solark_client:
                    app._print_status_display(app.solark_client.data)
                
                # Test SunSpec server
                if app.modbus_server and app.modbus_server.is_running():
                    print("✓ SunSpec server running")
                    stats = app.modbus_server.get_statistics()
                    print(f"Server stats: {stats}")
                
            else:
                print("✗ Sol-Ark poll failed")
                sys.exit(1)
            
            app.stop()
        else:
            # Normal operation
            app.run()
    
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()