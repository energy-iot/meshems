"""
EMS Main Application

Main entry point for the Energy Management System Python implementation.
Coordinates Sol-Ark data polling and SunSpec Modbus TCP server.
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

from .solark_client import SolArkModbusClient, SolArkData
from .modbus_server import SunSpecModbusServer, ModbusServerConfig
from .sunspec_models import SunSpecMapper


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
                    device_id=server_config.get("device_id", 1),
                    device_info=self.config.get("device_info", {})
                )
                
                self.modbus_server = SunSpecModbusServer(modbus_config)
                self.modbus_server.start()
            
            self.logger.info("All components initialized successfully")
            
        except Exception as e:
            self.logger.error(f"Error initializing components: {e}")
            raise
    
    def _create_status_display(self, solark_data: SolArkData) -> Layout:
        """Create rich status display"""
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
                f"[bold blue]EMS-Dev Python Gateway[/bold blue] - Sol-Ark Monitor",
                style="blue"
            )
        )
        
        # Main content - split into columns
        layout["main"].split_row(
            Layout(name="left"),
            Layout(name="right")
        )
        
        # Battery status table
        battery_table = Table(title="Battery Status", show_header=True, header_style="bold magenta")
        battery_table.add_column("Parameter", style="cyan")
        battery_table.add_column("Value", style="green")
        
        battery_table.add_row("Power", f"{solark_data.battery_power:.1f} W")
        battery_table.add_row("Current", f"{solark_data.battery_current:.2f} A")
        battery_table.add_row("Voltage", f"{solark_data.battery_voltage:.2f} V")
        battery_table.add_row("SOC", f"{solark_data.battery_soc:.0f}%")
        battery_table.add_row("Temperature", f"{solark_data.battery_temperature:.1f}°C")
        battery_table.add_row("Capacity", f"{solark_data.battery_capacity:.1f} Ah")
        
        # Status indicators
        status = "IDLE"
        if solark_data.battery_power < -50:
            status = "[green]CHARGING[/green]"
        elif solark_data.battery_power > 50:
            status = "[red]DISCHARGING[/red]"
        
        battery_table.add_row("Status", status)
        
        # Grid/Power table
        power_table = Table(title="Power & Grid", show_header=True, header_style="bold magenta")
        power_table.add_column("Parameter", style="cyan")
        power_table.add_column("Value", style="green")
        
        power_table.add_row("Grid Power", f"{solark_data.grid_power:.1f} W")
        power_table.add_row("Grid Voltage", f"{solark_data.grid_voltage:.1f} V")
        power_table.add_row("Grid Frequency", f"{solark_data.grid_frequency:.2f} Hz")
        power_table.add_row("Load Power", f"{solark_data.load_power_total:.1f} W")
        power_table.add_row("PV1 Power", f"{solark_data.pv1_power:.1f} W")
        power_table.add_row("PV2 Power", f"{solark_data.pv2_power:.1f} W")
        power_table.add_row("PV Total", f"{solark_data.pv_power_total:.3f} kW")
        
        # Grid status
        grid_status = "DISCONNECTED"
        if solark_data.grid_relay_status > 0:
            if solark_data.grid_power < -50:
                grid_status = "[green]SELLING[/green]"
            elif solark_data.grid_power > 50:
                grid_status = "[yellow]BUYING[/yellow]"
            else:
                grid_status = "[blue]CONNECTED[/blue]"
        
        power_table.add_row("Grid Status", grid_status)
        
        layout["left"].update(Panel(battery_table, border_style="blue"))
        layout["right"].update(Panel(power_table, border_style="green"))
        
        # Footer with timestamps
        footer_text = f"Last Update: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(solark_data.last_update))}"
        if self.modbus_server and self.modbus_server.is_running():
            footer_text += f" | SunSpec Server: [green]RUNNING[/green] on port {self.modbus_server.config.port}"
        else:
            footer_text += " | SunSpec Server: [red]STOPPED[/red]"
        
        layout["footer"].update(Panel(footer_text, style="dim"))
        
        return layout
    
    def _poll_solark(self):
        """Poll Sol-Ark inverter data"""
        try:
            if self.solark_client.poll():
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
                self.console.print("[bold green]EMS-Dev Python Gateway Started[/bold green]")
                self.console.print(f"Polling Sol-Ark every {poll_interval} seconds")
                if self.modbus_server:
                    self.console.print(f"SunSpec server running on port {self.modbus_server.config.port}")
            
            # Main loop
            while self.running:
                start_time = time.time()
                
                # Poll Sol-Ark data
                poll_success = self._poll_solark()
                
                # Update console display
                if console_output and (time.time() - last_console_update) >= console_update_interval:
                    if poll_success:
                        layout = self._create_status_display(self.solark_client.data)
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
        
        # Disconnect Sol-Ark client
        if self.solark_client:
            self.solark_client.disconnect()
        
        self.logger.info("EMS application stopped")


@click.command()
@click.option('--config', '-c', default='config.yaml', help='Configuration file path')
@click.option('--verbose', '-v', is_flag=True, help='Enable verbose logging')
@click.option('--test', '-t', is_flag=True, help='Test mode - single poll and exit')
def main(config: str, verbose: bool, test: bool):
    """
    EMS-Dev Python Gateway
    
    Energy Management System for Sol-Ark inverters with SunSpec Modbus TCP server.
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
            
            if app._poll_solark():
                console.print("[green]✓ Sol-Ark poll successful[/green]")
                
                # Display data
                layout = app._create_status_display(app.solark_client.data)
                console.print(layout)
                
                # Test SunSpec server
                if app.modbus_server and app.modbus_server.is_running():
                    console.print("[green]✓ SunSpec server running[/green]")
                    stats = app.modbus_server.get_statistics()
                    console.print(f"Server stats: {stats}")
                
            else:
                console.print("[red]✗ Sol-Ark poll failed[/red]")
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