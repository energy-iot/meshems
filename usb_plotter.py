#!/usr/bin/env python3
"""
MODBUS Current Monitor via USB
------------------------------
This script reads current data from the ESP32 via USB and plots it in real-time.

Requirements:
- Python 3.6 or higher
- pyserial
- matplotlib
- numpy

Install with:
pip install pyserial matplotlib numpy

Usage:
python usb_plotter.py [COM_PORT]

Example:
python usb_plotter.py /dev/tty.usbserial-0001
"""

import sys
import argparse
import serial
import time
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation
from collections import deque

# Default values
DEFAULT_BAUDRATE = 115200
DEFAULT_PORT = "/dev/ttyUSB0"  # For Linux
MAX_POINTS = 100  # Maximum number of points to display on the plot

# Parse command line arguments
parser = argparse.ArgumentParser(description='Plot MODBUS current data from ESP32 via USB')
parser.add_argument('port', nargs='?', default=DEFAULT_PORT, help='Serial port (e.g., COM3, /dev/ttyUSB0)')
parser.add_argument('--baudrate', type=int, default=DEFAULT_BAUDRATE, help='Baud rate')
args = parser.parse_args()

# Create data structures to store the data
times = deque(maxlen=MAX_POINTS)
currents = deque(maxlen=MAX_POINTS)
start_time = None  # To store the initial timestamp

# Set up the plot
plt.style.use('ggplot')
fig, ax = plt.subplots(figsize=(10, 6))
line, = ax.plot([], [], 'b-', lw=2)
ax.set_title('MODBUS Current Monitor')
ax.set_xlabel('Time (seconds)')
ax.set_ylabel('Current (A)')
ax.grid(True)

# Add a text annotation for the current value
current_text = ax.text(0.02, 0.95, '', transform=ax.transAxes, fontsize=14)

def init():
    """Initialize the plot"""
    line.set_data([], [])
    return line,

def update(frame):
    """Update the plot with new data"""
    global start_time
    
    # Read from the serial port
    try:
        while ser.in_waiting:
            raw_line = ser.readline()
            try:
                line_str = raw_line.decode('utf-8').strip()
                print(f"Received: {line_str}")  # Print every line received for debugging
                
                if line_str.startswith('DATA'):
                    # Parse the CSV data
                    parts = line_str.split(',')
                    if len(parts) >= 3:
                        try:
                            timestamp = int(parts[1]) / 1000.0  # Convert ms to seconds
                            current = float(parts[2])
                            
                            # Initialize start_time if not set
                            if start_time is None:
                                start_time = timestamp
                            
                            # Calculate relative time
                            relative_time = timestamp - start_time
                            
                            print(f"Parsed data: time={relative_time:.2f}s, current={current}A")
                            
                            # Add to our data structures
                            times.append(relative_time)
                            currents.append(current)
                            
                            # Update the current value text
                            current_text.set_text(f'Current: {current:.3f} A')
                        except ValueError as e:
                            print(f"Error parsing values: {e}")
            except UnicodeDecodeError:
                print(f"Received binary data: {raw_line}")
    except Exception as e:
        print(f"Error reading serial port: {e}")
    
    # Auto-scale the y-axis if needed
    if currents:
        min_current = max(0, min(currents) * 0.9)  # Add some padding
        max_current = max(currents) * 1.1  # Add some padding
        
        # Ensure min and max are different
        if max_current <= min_current:
            max_current = min_current + 1.0
            
        ax.set_ylim(min_current, max_current)
    
    # Update the line data
    if times and currents:
        # Update x-axis limits to show a moving window
        current_time = times[-1]
        ax.set_xlim(max(0, current_time - 30), current_time + 2)  # Show last 30 seconds + 2 second padding
        
        # Update the line with the actual timestamps
        line.set_data(list(times), list(currents))
    
    return line, current_text

# Open the serial port
try:
    # First check if we need to close any existing connections to the port
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    
    # Print all available ports for debugging
    print("Available ports:")
    for port in ports:
        print(f" - {port.device} ({port.description})")
    
    ser = serial.Serial(args.port, args.baudrate, timeout=1)
    # Set DTR and RTS to control ESP32 reset behavior (might help with stability)
    ser.dtr = False
    ser.rts = False
    
    # Flush any existing data
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    
    print(f"Connected to {args.port} at {args.baudrate} baud")
except Exception as e:
    print(f"Error opening serial port {args.port}: {e}")
    available_ports = []
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    for port in ports:
        available_ports.append(port.device)
    if available_ports:
        print(f"Available ports: {', '.join(available_ports)}")
        print(f"Try using one of these with: python {sys.argv[0]} PORT_NAME")
    sys.exit(1)

# Start the animation
ani = FuncAnimation(fig, update, init_func=init, interval=100, blit=True)

try:
    plt.tight_layout()
    plt.show()
except KeyboardInterrupt:
    print("Exiting...")
finally:
    # Close the serial port
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.") 