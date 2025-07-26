# EMS-Dev Python Gateway

A Python implementation of the Energy Management System (EMS) for Sol-Ark inverters, providing SunSpec-compliant Modbus TCP server functionality on Linux systems using `/dev/tty` RS485 ports.

## Overview

This project is a Python port of the original C++/Arduino EMS firmware, designed to run on Linux systems. It provides:

- **Modbus RTU Client**: Communicates with Sol-Ark inverters via RS485
- **SunSpec Modbus TCP Server**: Exposes inverter data in standardized SunSpec format
- **Real-time Monitoring**: Console display with live data updates
- **Data Logging**: Historical data storage and export capabilities
- **Configurable**: YAML-based configuration system

## Features

### Sol-Ark Integration
- Full register mapping for Sol-Ark LV inverters
- Battery, grid, PV, and load monitoring
- BMS data support for lithium batteries
- Diagnostic information and status monitoring

### SunSpec Compliance
- Common Model (1) - Device identification
- Inverter Model (701) - Single phase with split phase output
- Battery Model (713) - Battery bank monitoring
- Standard Modbus TCP server on port 8502

### Linux Compatibility
- Uses `/dev/tty` serial ports for RS485 communication
- Systemd service support
- Configurable serial parameters
- Signal handling for graceful shutdown

## Requirements

- Python 3.8 or higher
- Linux operating system
- RS485 to USB adapter or built-in RS485 port
- Sol-Ark inverter with Modbus RTU support

## Installation

### From Source

1. Clone the repository:
```bash
git clone https://github.com/energy-iot/ems-dev-python.git
cd ems-dev-python
```

2. Install dependencies:
```bash
pip install -r requirements.txt
```

3. Install the package:
```bash
pip install -e .
```

### Using pip

```bash
pip install ems-dev-python
```

## Configuration

Create a `config.yaml` file with your settings:

```yaml
# Serial/RS485 Configuration
serial:
  port: "/dev/ttyUSB0"  # Your RS485 device
  baudrate: 9600
  timeout: 1.0

# Sol-Ark Inverter Configuration
solark:
  modbus_address: 1
  poll_interval: 5.0  # seconds

# SunSpec Modbus TCP Server
sunspec_server:
  enabled: true
  host: "0.0.0.0"
  port: 8502

# Device Information
device_info:
  manufacturer: "Energy IoT Open Source"
  model: "EMS-Dev Python"
  version: "1.0.0"
  serial_number: "EMS-PY-001"
```

## Usage

### Command Line

Run the gateway:
```bash
ems-dev --config config.yaml
```

Test connection:
```bash
ems-dev --config config.yaml --test
```

Enable verbose logging:
```bash
ems-dev --config config.yaml --verbose
```

### Python API

```python
from ems import EMSApplication

# Create and run application
app = EMSApplication("config.yaml")
app.run()
```

### SunSpec Client Example

Connect to the SunSpec server:

```python
import sunspec2.modbus.client as client

# Connect to the gateway
device = client.SunSpecModbusClientDeviceTCP(
    ipaddr="192.168.1.100",
    ipport=8502
)

# Scan for models
device.scan()

# Read inverter data
if 701 in device.models:
    inverter = device.models[701]
    inverter.read()
    print(f"AC Power: {inverter.W.value} W")
    print(f"Battery SOC: {inverter.DCV.value} V")
```

## Hardware Setup

### RS485 Connection

Connect your RS485 adapter to the Sol-Ark inverter:

1. **A+** (Data+) - Connect to Sol-Ark A+ terminal
2. **B-** (Data-) - Connect to Sol-Ark B- terminal
3. **GND** - Connect to Sol-Ark ground if available

### Termination

Add a 120Ω termination resistor across A+ and B- at the far end of the RS485 bus.

### USB Permissions

Add your user to the dialout group:
```bash
sudo usermod -a -G dialout $USER
```

Or set permissions for the device:
```bash
sudo chmod 666 /dev/ttyUSB0
```

## Systemd Service

Create a systemd service for automatic startup:

```ini
[Unit]
Description=EMS-Dev Python Gateway
After=network.target

[Service]
Type=simple
User=ems
WorkingDirectory=/opt/ems-dev
ExecStart=/usr/local/bin/ems-dev --config /opt/ems-dev/config.yaml
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable ems-dev
sudo systemctl start ems-dev
```

## Monitoring

### Console Output

The application provides a rich console interface showing:
- Battery status (power, voltage, SOC, temperature)
- Grid status (power, voltage, frequency)
- PV generation (PV1, PV2, total)
- Load consumption
- System status and timestamps

### Web Interface

Access the SunSpec data via Modbus TCP:
- **Host**: Your system's IP address
- **Port**: 8502 (configurable)
- **Unit ID**: 1 (configurable)

### Data Export

Historical data can be exported to:
- SQLite database
- CSV files
- JSON format

## Troubleshooting

### Serial Port Issues

Check device permissions:
```bash
ls -l /dev/ttyUSB*
sudo dmesg | grep tty
```

Test serial communication:
```bash
sudo minicom -D /dev/ttyUSB0 -b 9600
```

### Modbus Communication

Enable debug logging:
```bash
ems-dev --config config.yaml --verbose
```

Check Sol-Ark settings:
- Modbus RTU enabled
- Correct baud rate (9600)
- Correct slave address (default: 1)

### Network Issues

Test SunSpec server:
```bash
telnet localhost 8502
```

Check firewall settings:
```bash
sudo ufw allow 8502
```

## Development

### Project Structure

```
ems/
├── __init__.py          # Package initialization
├── main.py              # Main application entry point
├── solark_client.py     # Sol-Ark Modbus RTU client
├── solark_registers.py  # Register mappings and constants
├── sunspec_models.py    # SunSpec model implementations
└── modbus_server.py     # SunSpec Modbus TCP server
```

### Running Tests

```bash
python -m pytest tests/
```

### Code Style

```bash
black ems/
flake8 ems/
mypy ems/
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Original C++ EMS firmware by Doug Mendonca and Liam O'Brien
- SunSpec Alliance for standardized solar data models
- pymodbus and pysunspec2 library maintainers

## Support

- **Issues**: [GitHub Issues](https://github.com/energy-iot/ems-dev-python/issues)
- **Documentation**: [Wiki](https://github.com/energy-iot/ems-dev-python/wiki)
- **Community**: [Discussions](https://github.com/energy-iot/ems-dev-python/discussions)
