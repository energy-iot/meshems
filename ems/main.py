"""
EMS Main Application

Main entry point for the Energy Management System Python implementation.
Coordinates inverter data polling and SunSpec Modbus TCP server.
"""

import asyncio
import logging
import signal
import sys
import time
import yaml
from pathlib import Path
from typing import Dict, Any, Optional

import click
from rich.console import Console
from rich.table import Table
from rich.live import Live
from rich.layout import Layout
from rich.panel import Panel

from .inverter_factory import InverterFactory
from .modbus_server import SunSpecModbusServer, ModbusServerConfig
from .sunspec_models import SunSpecMapper
from .base import InverterData


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
        self.inverter_client = None
        self.modbus_server: Optional[SunSpecModbusServer] = None
        
        # Console for rich output
        self.console = Console()
        
        # Load configuration
        self._load_config()
        
        # Setup logging
        self._setup_logging()
        
        self.logger = logging.getLogger(__name__)
        self.logger.info("EMS Application initialized")
    
    def _load_config(self):
        """Load configuration from YAML file"""
        try:
            config_file = Path(self.config_path)
            if config_file.exists():
                with open(config_file, 'r') as f:
                    self.config = yaml.safe_load(f)
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
            "inverter": {
                "type": "solark",
                "modbus_address": 1,
                "poll_interval": 5.0,
                "max_retries": 3,
                "retry_delay": 0.5,
                "generic": {
                    "register_mapping_file": "generic_registers.json"
                }
            },
            "sunspec_server": {
                "enabled": True,
                "host": "0.0.0.0",
                "port": 8502,
                "device_id": 1
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
                "options": "Generic Inverter Gateway"
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
            # Initialize inverter client
            serial_config = self.config.get("serial", {})
            inverter_config = self.config.get("inverter", {})
            
            inverter_type = inverter_config.get("type", "solark")
            modbus_address = inverter_config.get("modbus_address", 1)
            
            # Prepare factory arguments
            factory_args = {
                "port": serial_config.get("port", "/dev/ttyUSB0"),
                "baudrate": serial_config.get("baudrate", 9600),
                "modbus_address": modbus_address
            }
            
            # Add inverter-specific configuration - all inverters now use JSON mapping
            if inverter_type == "solark":
                solark_config = inverter_config.get("solark", {})
                factory_args["config_file"] = solark_config.get("register_mapping_file", "solark_registers.json")
            elif inverter_type == "generic":
                generic_config = inverter_config.get("generic", {})
                factory_args["config_file"] = generic_config.get("register_mapping_file", "generic_registers.json")
            
            # Create inverter client using factory
            self.inverter_client = InverterFactory.create_inverter(inverter_type, **factory_args)
            
            if self.inverter_client is None:
                raise Exception(f"Failed to create inverter client for type: {inverter_type}")
            
            # Connect to inverter
            if not self.inverter_client.connect():
                raise Exception(f"Failed to connect to {inverter_type} inverter")
            
            # Initialize Modbus server if enabled
            server_config = self.config.get("sunspec_server", {})
            if server_config.get("enabled", True):
                modbus_config = ModbusServerConfig(
                    host=server_config.get("host", "0.0.0.0"),
                    port=server_config.get("port", 8502),
                    device_id=server_config.get("device_id", 1),
                    device_info=self.config.get("device_info", {})
                )
                
                self.modbus_server = SunSpecModbusServer(modbus_config)
                self.modbus_server.start()
            
            self.logger.info("All components initialized successfully")
            
        except Exception as e:
            self.logger.error(f"Error initializing components: {e}")
            raise
    
    def _create_status_display(self, inverter_data: InverterData) -> Layout:
        """Create rich status display with all available data values"""
        layout = Layout()
        
        # Create main sections
        layout.split_column(
            Layout(name="header", size=3),
            Layout(name="main"),
            Layout(name="footer", size=3)
        )
        
        # Header
        layout["header"].update(
            Panel(
                f"[bold blue]EMS-Dev Python Gateway[/bold blue] - {self.config.get('inverter', {}).get('type', 'Generic')} Inverter Monitor",
                style="blue"
            )
        )
        
        # Main content - split into multiple rows for better organization
        layout["main"].split_column(
            Layout(name="row1", size=12),
            Layout(name="row2", size=12),
            Layout(name="row3", size=12),
            Layout(name="row4", size=12)
        )
        
        # Row 1 - Battery and BMS data
        layout["row1"].split_row(
            Layout(name="battery"),
            Layout(name="bms")
        )
        
        # Row 2 - Power/Grid and Inverter data
        layout["row2"].split_row(
            Layout(name="power"),
            Layout(name="inverter")
        )
        
        # Row 3 - Energy counters and Diagnostics
        layout["row3"].split_row(
            Layout(name="energy"),
            Layout(name="diagnostics")
        )
        
        # Row 4 - Additional data
        layout["row4"].split_row(
            Layout(name="additional_left"),
            Layout(name="additional_right")
        )
        
        # Battery status table
        battery_table = Table(title="Battery Status", show_header=True, header_style="bold magenta")
        battery_table.add_column("Parameter", style="cyan")
        battery_table.add_column("Value", style="green")
        
        battery_table.add_row("Power", f"{getattr(inverter_data, 'battery_power', 0):.1f} W")
        battery_table.add_row("Current", f"{getattr(inverter_data, 'battery_current', 0):.2f} A")
        battery_table.add_row("Voltage", f"{getattr(inverter_data, 'battery_voltage', 0):.2f} V")
        battery_table.add_row("SOC", f"{getattr(inverter_data, 'battery_soc', 0):.0f}%")
        battery_table.add_row("Temperature", f"{getattr(inverter_data, 'battery_temperature', 0):.1f}°C")
        battery_table.add_row("Capacity", f"{getattr(inverter_data, 'battery_capacity', 0):.1f} Ah")
        
        # Status indicators
        status = "IDLE"
        if hasattr(inverter_data, 'battery_power'):
            if inverter_data.battery_power < -50:
                status = "[green]CHARGING[/green]"
            elif inverter_data.battery_power > 50:
                status = "[red]DISCHARGING[/red]"
        
        battery_table.add_row("Status", status)
        
        # BMS data table (if available)
        bms_table = Table(title="BMS Data", show_header=True, header_style="bold magenta")
        bms_table.add_column("Parameter", style="cyan")
        bms_table.add_column("Value", style="green")
        
        if hasattr(inverter_data, 'bms_real_time_soc'):
            bms_table.add_row("SOC", f"{inverter_data.bms_real_time_soc:.0f}%")
        if hasattr(inverter_data, 'bms_real_time_voltage'):
            bms_table.add_row("Voltage", f"{inverter_data.bms_real_time_voltage:.2f} V")
        if hasattr(inverter_data, 'bms_real_time_current'):
            bms_table.add_row("Current", f"{inverter_data.bms_real_time_current:.2f} A")
        if hasattr(inverter_data, 'bms_real_time_temp'):
            bms_table.add_row("Temperature", f"{inverter_data.bms_real_time_temp:.1f}°C")
        if hasattr(inverter_data, 'bms_warning'):
            bms_table.add_row("Warning", f"{inverter_data.bms_warning}")
        if hasattr(inverter_data, 'bms_fault'):
            bms_table.add_row("Fault", f"{inverter_data.bms_fault}")
        
        # Grid/Power table
        power_table = Table(title="Power & Grid", show_header=True, header_style="bold magenta")
        power_table.add_column("Parameter", style="cyan")
        power_table.add_column("Value", style="green")
        
        power_table.add_row("Grid Power", f"{getattr(inverter_data, 'grid_power', 0):.1f} W")
        power_table.add_row("Grid Voltage (L1-L2)", f"{getattr(inverter_data, 'grid_voltage_l1l2', 0):.1f} V")
        power_table.add_row("Grid Voltage (L1-N)", f"{getattr(inverter_data, 'grid_voltage_l1n', 0):.1f} V")
        power_table.add_row("Grid Voltage (L2-N)", f"{getattr(inverter_data, 'grid_voltage_l2n', 0):.1f} V")
        power_table.add_row("Grid Current L1", f"{getattr(inverter_data, 'grid_current_l1', 0):.2f} A")
        power_table.add_row("Grid Current L2", f"{getattr(inverter_data, 'grid_current_l2', 0):.2f} A")
        power_table.add_row("Grid Frequency", f"{getattr(inverter_data, 'grid_frequency', 0):.2f} Hz")
        power_table.add_row("Load Power", f"{getattr(inverter_data, 'load_power_total', 0):.1f} W")
        power_table.add_row("Load Power L1", f"{getattr(inverter_data, 'load_power_l1', 0):.1f} W")
        power_table.add_row("Load Power L2", f"{getattr(inverter_data, 'load_power_l2', 0):.1f} W")
        power_table.add_row("PV1 Power", f"{getattr(inverter_data, 'pv1_power', 0):.1f} W")
        power_table.add_row("PV2 Power", f"{getattr(inverter_data, 'pv2_power', 0):.1f} W")
        power_table.add_row("Apparent Power", f"{getattr(inverter_data, 'apparent_power', 0):.1f} VA")
        power_table.add_row("Power Factor", f"{getattr(inverter_data, 'grid_power_factor', 0):.2f}")
        
        # Grid status
        grid_status = "DISCONNECTED"
        if hasattr(inverter_data, 'grid_relay_status') and inverter_data.grid_relay_status > 0:
            if hasattr(inverter_data, 'grid_power'):
                if inverter_data.grid_power < -50:
                    grid_status = "[green]SELLING[/green]"
                elif inverter_data.grid_power > 50:
                    grid_status = "[yellow]BUYING[/yellow]"
                else:
                    grid_status = "[blue]CONNECTED[/blue]"
        
        power_table.add_row("Grid Status", grid_status)
        
        # Inverter data table
        inverter_table = Table(title="Inverter Data", show_header=True, header_style="bold magenta")
        inverter_table.add_column("Parameter", style="cyan")
        inverter_table.add_column("Value", style="green")
        
        inverter_table.add_row("Output Power", f"{getattr(inverter_data, 'inverter_output_power', 0):.1f} W")
        inverter_table.add_row("Voltage (L1-L2)", f"{getattr(inverter_data, 'inverter_voltage', 0):.1f} V")
        inverter_table.add_row("Voltage (L1-N)", f"{getattr(inverter_data, 'inverter_voltage_ln', 0):.1f} V")
        inverter_table.add_row("Voltage (L2-N)", f"{getattr(inverter_data, 'inverter_voltage_l2n', 0):.1f} V")
        inverter_table.add_row("Current L1", f"{getattr(inverter_data, 'inverter_current_l1', 0):.2f} A")
        inverter_table.add_row("Current L2", f"{getattr(inverter_data, 'inverter_current_l2', 0):.2f} A")
        inverter_table.add_row("Frequency", f"{getattr(inverter_data, 'inverter_frequency', 0):.2f} Hz")
        inverter_table.add_row("Status", f"{getattr(inverter_data, 'inverter_status', 0)}")
        inverter_table.add_row("Power L1", f"{getattr(inverter_data, 'inverter_power_l1', 0):.1f} W")
        inverter_table.add_row("Power L2", f"{getattr(inverter_data, 'inverter_power_l2', 0):.1f} W")
        
        # Energy counters table
        energy_table = Table(title="Energy Counters", show_header=True, header_style="bold magenta")
        energy_table.add_column("Parameter", style="cyan")
        energy_table.add_column("Value", style="green")
        
        energy_table.add_row("Grid Buy", f"{getattr(inverter_data, 'grid_buy_energy', 0):.2f} kWh")
        energy_table.add_row("Grid Sell", f"{getattr(inverter_data, 'grid_sell_energy', 0):.2f} kWh")
        energy_table.add_row("Load", f"{getattr(inverter_data, 'load_energy', 0):.2f} kWh")
        energy_table.add_row("PV", f"{getattr(inverter_data, 'pv_energy', 0):.2f} kWh")
        
        # Diagnostic data table
        diag_table = Table(title="Diagnostics", show_header=True, header_style="bold magenta")
        diag_table.add_column("Parameter", style="cyan")
        diag_table.add_column("Value", style="green")
        
        if hasattr(inverter_data, 'comm_version'):
            diag_table.add_row("Comm Version", f"{inverter_data.comm_version}")
        if hasattr(inverter_data, 'igbt_temp'):
            diag_table.add_row("IGBT Temp", f"{inverter_data.igbt_temp:.1f}°C")
        if hasattr(inverter_data, 'dcdc_xfrmr_temp'):
            diag_table.add_row("DCDC XFRMR Temp", f"{inverter_data.dcdc_xfrmr_temp:.1f}°C")
        if hasattr(inverter_data, 'grid_type'):
            diag_table.add_row("Grid Type", f"{inverter_data.grid_type}")
        if hasattr(inverter_data, 'generator_relay_status'):
            diag_table.add_row("Generator Relay", f"{inverter_data.generator_relay_status}")
        
        # Additional data tables
        additional_left_table = Table(title="Load Data", show_header=True, header_style="bold magenta")
        additional_left_table.add_column("Parameter", style="cyan")
        additional_left_table.add_column("Value", style="green")
        
        additional_left_table.add_row("Load Current L1", f"{getattr(inverter_data, 'load_current_l1', 0):.2f} A")
        additional_left_table.add_row("Load Current L2", f"{getattr(inverter_data, 'load_current_l2', 0):.2f} A")
        additional_left_table.add_row("Load Frequency", f"{getattr(inverter_data, 'load_frequency', 0):.2f} Hz")
        
        additional_right_table = Table(title="System Info", show_header=True, header_style="bold magenta")
        additional_right_table.add_column("Parameter", style="cyan")
        additional_right_table.add_column("Value", style="green")
        
        # Update layout with all tables
        layout["battery"].update(Panel(battery_table, border_style="blue"))
        layout["bms"].update(Panel(bms_table, border_style="magenta"))
        layout["power"].update(Panel(power_table, border_style="green"))
        layout["inverter"].update(Panel(inverter_table, border_style="yellow"))
        layout["energy"].update(Panel(energy_table, border_style="cyan"))
        layout["diagnostics"].update(Panel(diag_table, border_style="white"))
        layout["additional_left"].update(Panel(additional_left_table, border_style="red"))
        layout["additional_right"].update(Panel(additional_right_table, border_style="purple"))
        
        # Footer with timestamps
        footer_text = f"Last Update: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(getattr(inverter_data, 'last_update', time.time())))}"
        if self.modbus_server and self.modbus_server.is_running():
            footer_text += f" | SunSpec Server: [green]RUNNING[/green] on port {self.modbus_server.config.port}"
        else:
            footer_text += " | SunSpec Server: [red]STOPPED[/red]"
        
        layout["footer"].update(Panel(footer_text, style="dim"))
        
        return layout
    
    def _poll_inverter(self):
        """Poll inverter data"""
        try:
            if self.inverter_client.poll():
                # Update Modbus server if running
                if self.modbus_server:
                    self.modbus_server.update_from_inverter(self.inverter_client.data)
                
                return True
            else:
                self.logger.warning("Failed to poll inverter data")
                return False
                
        except Exception as e:
            self.logger.error(f"Error polling inverter: {e}")
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
            inverter_config = self.config.get("inverter", {})
            poll_interval = inverter_config.get("poll_interval", 5.0)
            console_output = self.config.get("monitoring", {}).get("console_output", True)
            console_update_interval = self.config.get("monitoring", {}).get("console_update_interval", 10.0)
            
            last_console_update = 0
            
            if console_output:
                self.console.print("[bold green]EMS-Dev Python Gateway Started[/bold green]")
                self.console.print(f"Polling inverter every {poll_interval} seconds")
                if self.modbus_server:
                    self.console.print(f"SunSpec server running on port {self.modbus_server.config.port}")
            
            # Main loop
            while self.running:
                start_time = time.time()
                
                # Poll inverter data
                poll_success = self._poll_inverter()
                
                # Update console display
                if console_output and (time.time() - last_console_update) >= console_update_interval:
                    if poll_success and self.inverter_client:
                        layout = self._create_status_display(self.inverter_client.data)
                        self.console.clear()
                        self.console.print(layout)
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
        
        # Disconnect inverter client
        if self.inverter_client:
            self.inverter_client.disconnect()
        
        self.logger.info("EMS application stopped")


@click.command()
@click.option('--config', '-c', default='config.yaml', help='Configuration file path')
@click.option('--verbose', '-v', is_flag=True, help='Enable verbose logging')
@click.option('--test', '-t', is_flag=True, help='Test mode - single poll and exit')
def main(config: str, verbose: bool, test: bool):
    """
    EMS-Dev Python Gateway
    
    Energy Management System for inverters with SunSpec Modbus TCP server.
    """
    console = Console()
    
    try:
        # Create application
        app = EMSApplication(config)
        
        # Override logging level if verbose
        if verbose:
            logging.getLogger().setLevel(logging.DEBUG)
        
        if test:
            # Test mode - single poll
            console.print("[yellow]Running in test mode...[/yellow]")
            app._initialize_components()
            
            if app._poll_inverter():
                console.print("[green]✓ Inverter poll successful[/green]")
                
                # Display data
                if app.inverter_client:
                    layout = app._create_status_display(app.inverter_client.data)
                    console.print(layout)
                
                # Test SunSpec server
                if app.modbus_server and app.modbus_server.is_running():
                    console.print("[green]✓ SunSpec server running[/green]")
                    stats = app.modbus_server.get_statistics()
                    console.print(f"Server stats: {stats}")
                
            else:
                console.print("[red]✗ Inverter poll failed[/red]")
                sys.exit(1)
            
            app.stop()
        else:
            # Normal operation
            app.run()

    except Exception as e:
        console.print(f"[red]Error: {e}[/red]")
        sys.exit(1)


if __name__ == "__main__":
    main()