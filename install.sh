#!/bin/bash
# Installation script for EMS-Dev Python Gateway

set -e

echo "EMS-Dev Python Gateway Installation Script"
echo "=========================================="

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "This script should not be run as root. Please run as a regular user."
   exit 1
fi

# Check Python version
echo "Checking Python version..."
python_version=$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[:2])))')
required_version="3.8"

if python3 -c "import sys; exit(0 if sys.version_info >= (3, 8) else 1)"; then
    echo "✓ Python $python_version found (>= $required_version required)"
else
    echo "✗ Python $required_version or higher is required. Found: $python_version"
    echo "Please install Python 3.8+ and try again."
    exit 1
fi

# Check if pip is available
if ! command -v pip3 &> /dev/null; then
    echo "✗ pip3 not found. Please install pip3 and try again."
    exit 1
fi

echo "✓ pip3 found"

# Create virtual environment
echo "Creating virtual environment..."
if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo "✓ Virtual environment created"
else
    echo "✓ Virtual environment already exists"
fi

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate

# Upgrade pip
echo "Upgrading pip..."
pip install --upgrade pip

# Install dependencies
echo "Installing dependencies..."
pip install -r requirements.txt

# Install package in development mode
echo "Installing EMS-Dev Python package..."
pip install -e .

# Check serial port permissions
# echo "Checking serial port permissions..."
# if groups $USER | grep -q dialout; then
#     echo "✓ User is in dialout group"
# else
#     echo "⚠ User is not in dialout group. Adding user to dialout group..."
#     echo "You may need to enter your password:"
#     sudo usermod -a -G dialout $USER
#     echo "✓ User added to dialout group"
#     echo "⚠ You need to log out and log back in for group changes to take effect"
# fi

# # Check for USB serial devices
# echo "Checking for USB serial devices..."
# if ls /dev/ttyUSB* 1> /dev/null 2>&1; then
#     echo "✓ Found USB serial devices:"
#     ls -l /dev/ttyUSB*
# else
#     echo "⚠ No USB serial devices found (/dev/ttyUSB*)"
#     echo "  Make sure your RS485 adapter is connected"
# fi

# Create default configuration if it doesn't exist
if [ ! -f "config.yaml" ]; then
    echo "Creating default configuration..."
    cat > config.yaml << 'EOF'
# EMS Python Configuration
serial:
  port: "/dev/ttyRS485"
  baudrate: 9600
  timeout: 2.0

solark:
  modbus_address: 1
  poll_interval: 5.0
  max_retries: 3

sunspec_server:
  enabled: true
  host: "0.0.0.0"
  port: 8502
  unit_id: 1

logging:
  level: "INFO"
  file: "ems.log"

device_info:
  manufacturer: "Energy IoT Open Source"
  model: "EMS-Dev Python"
  version: "1.0.0"
  serial_number: "EMS-PY-001"
  options: "Sol-Ark Gateway"

monitoring:
  console_output: true
  console_update_interval: 10.0
EOF
    echo "✓ Default configuration created (config.yaml)"
else
    echo "✓ Configuration file already exists"
fi

# Create systemd service file
echo "Creating systemd service file..."
cat > ems-dev.service << EOF
[Unit]
Description=EMS-Dev Python Gateway
After=network.target

[Service]
Type=simple
User=$USER
Group=$USER
WorkingDirectory=$(pwd)
Environment=PATH=$(pwd)/venv/bin
ExecStart=$(pwd)/venv/bin/ems-dev --config $(pwd)/config.yaml
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

echo "✓ Systemd service file created (ems-dev.service)"

# Make scripts executable
chmod +x examples/test_client.py
chmod +x examples/test_sunspec_server.py

echo ""
echo "Installation completed successfully!"
echo ""
echo "Next steps:"
echo "1. Edit config.yaml to match your setup (especially the serial port)"
echo "2. Test the connection: python examples/test_client.py --port /dev/ttyUSB0"
echo "3. Run the gateway: ems-dev --config config.yaml"
echo ""
echo "Optional - Install as system service:"
echo "  sudo cp ems-dev.service /etc/systemd/system/"
echo "  sudo systemctl enable ems-dev"
echo "  sudo systemctl start ems-dev"
echo ""
echo "For help and documentation, see README.md"