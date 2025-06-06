#!/usr/bin/env python3
"""
MODBUS Current Monitor via USB
------------------------------
This script reads electrical measurement data from the ESP32 via USB and plots it in real-time.

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
MAX_POINTS = 600  # 1 minute of data at 100ms sampling rate

# Define consistent colors for each measurement
COLORS = {
    'current': '#1f77b4',  # Blue
    'voltage': '#d62728',  # Red
    'power': '#2ca02c',    # Green
    'pf': '#9467bd',       # Purple
    'freq': '#ff7f0e'      # Orange
}

# Parse command line arguments
parser = argparse.ArgumentParser(description='Plot electrical measurement data from ESP32 via USB')
parser.add_argument('port', nargs='?', default=DEFAULT_PORT, help='Serial port (e.g., COM3, /dev/ttyUSB0)')
parser.add_argument('--baudrate', type=int, default=DEFAULT_BAUDRATE, help='Baud rate')
parser.add_argument('--interval', type=int, default=100, help='Plot update interval in milliseconds (default: 100)')
args = parser.parse_args()

# Create data structures to store the data
class DataBuffer:
    def __init__(self, maxlen=MAX_POINTS):
        self.times = deque(maxlen=maxlen)
        self.values = deque(maxlen=maxlen)
        self.start_time = None
        self.min_val = float('inf')
        self.max_val = float('-inf')

# Create buffers for each measurement
current_data = DataBuffer()
voltage_data = DataBuffer()
power_data = DataBuffer()
pf_data = DataBuffer()  # Power Factor
freq_data = DataBuffer()  # Frequency

# Set up the plot with subplots
plt.style.use('dark_background')  # Modern dark theme that's built into matplotlib
fig, (ax_current, ax_voltage, ax_power, ax_pf, ax_freq) = plt.subplots(5, 1, figsize=(12, 10), sharex=True)
fig.suptitle('Electrical Measurements Monitor', fontsize=14, y=0.995, color='white')

# Create line objects for each subplot with consistent colors
line_current, = ax_current.plot([], [], color=COLORS['current'], lw=2, label='Current')
line_voltage, = ax_voltage.plot([], [], color=COLORS['voltage'], lw=2, label='Voltage')
line_power, = ax_power.plot([], [], color=COLORS['power'], lw=2, label='Power')
line_pf, = ax_pf.plot([], [], color=COLORS['pf'], lw=2, label='Power Factor')
line_freq, = ax_freq.plot([], [], color=COLORS['freq'], lw=2, label='Frequency')

# Configure axes with improved styling
for ax in [ax_current, ax_voltage, ax_power, ax_pf, ax_freq]:
    ax.grid(True, linestyle='--', alpha=0.3)
    ax.set_facecolor('#1f1f1f')  # Darker background for better contrast

ax_current.set_ylabel('Current (A)', color=COLORS['current'], fontweight='bold')
ax_voltage.set_ylabel('Voltage (V)', color=COLORS['voltage'], fontweight='bold')
ax_power.set_ylabel('Power (W)', color=COLORS['power'], fontweight='bold')
ax_pf.set_ylabel('Power Factor', color=COLORS['pf'], fontweight='bold')
ax_freq.set_ylabel('Frequency (Hz)', color=COLORS['freq'], fontweight='bold')
ax_freq.set_xlabel('Time (seconds)', fontweight='bold')

# Add text annotations for current values with matching colors
text_current = ax_current.text(0.02, 0.95, '', transform=ax_current.transAxes, color=COLORS['current'], fontweight='bold')
text_voltage = ax_voltage.text(0.02, 0.95, '', transform=ax_voltage.transAxes, color=COLORS['voltage'], fontweight='bold')
text_power = ax_power.text(0.02, 0.95, '', transform=ax_power.transAxes, color=COLORS['power'], fontweight='bold')
text_pf = ax_pf.text(0.02, 0.95, '', transform=ax_pf.transAxes, color=COLORS['pf'], fontweight='bold')
text_freq = ax_freq.text(0.02, 0.95, '', transform=ax_freq.transAxes, color=COLORS['freq'], fontweight='bold')

def update_data_buffer(buffer, timestamp, value):
    """Update a data buffer with new values"""
    if buffer.start_time is None:
        buffer.start_time = timestamp
    
    relative_time = timestamp - buffer.start_time
    buffer.times.append(relative_time)
    buffer.values.append(value)
    
    # Update min/max values
    buffer.min_val = min(buffer.min_val, value)
    buffer.max_val = max(buffer.max_val, value)

def update_plot(ax, line, text, data, label):
    """Update a subplot with new data"""
    if data.values:
        # Get the current time window
        current_time = data.times[-1]
        time_window_start = max(0, current_time - 60)  # Last 60 seconds
        
        # Find data points within the current time window
        visible_data = []
        for t, v in zip(data.times, data.values):
            if t >= time_window_start:
                visible_data.append(v)
        
        if visible_data:
            # Calculate dynamic y-axis limits based on visible data
            data_min = min(visible_data)
            data_max = max(visible_data)
            data_range = data_max - data_min if data_max != data_min else 1.0
            
            # Add padding based on data type
            if "Current" in label:
                padding = max(data_range * 0.2, 0.1)
                ymin = data_min - padding
                ymax = data_max + padding
            elif "Voltage" in label:
                padding = max(data_range * 0.1, 1.0)
                ymin = data_min - padding
                ymax = data_max + padding
            elif "Power" in label:
                padding = max(data_range * 0.2, 10.0)
                ymin = data_min - padding
                ymax = data_max + padding
            elif "PF" in label:
                padding = max(data_range * 0.1, 0.05)
                ymin = max(0, data_min - padding)
                ymax = min(1, data_max + padding)
            elif "Freq" in label:
                padding = max(data_range * 0.1, 0.1)
                ymin = data_min - padding
                ymax = data_max + padding
            
            # Update y-axis limits
            ax.set_ylim(ymin, ymax)
            
            # Clear existing ticks and labels
            ax.yaxis.set_major_locator(plt.AutoLocator())
            
            # Set appropriate number format based on the range
            if "PF" in label:
                ax.yaxis.set_major_formatter(plt.FormatStrFormatter('%.2f'))
            else:
                # Determine format based on the range
                if data_range < 0.1:
                    ax.yaxis.set_major_formatter(plt.FormatStrFormatter('%.3f'))
                elif data_range < 1:
                    ax.yaxis.set_major_formatter(plt.FormatStrFormatter('%.2f'))
                elif data_range < 10:
                    ax.yaxis.set_major_formatter(plt.FormatStrFormatter('%.1f'))
                else:
                    ax.yaxis.set_major_formatter(plt.FormatStrFormatter('%.0f'))
            
            # Force update of tick labels
            ax.relim()
            ax.autoscale_view()
        
        # Update line data
        line.set_data(list(data.times), list(data.values))
        
        # Update current value text
        if "PF" in label:
            text.set_text(f'{label}: {data.values[-1]:.2f}')
        else:
            text.set_text(f'{label}: {data.values[-1]:.1f}')

def init():
    """Initialize the plot"""
    for line in [line_current, line_voltage, line_power, line_pf, line_freq]:
        line.set_data([], [])
    return line_current, line_voltage, line_power, line_pf, line_freq

def update(frame):
    """Update the plot with new data"""
    try:
        # Read all available data without blocking
        while ser.in_waiting:
            raw_line = ser.readline()
            try:
                line_str = raw_line.decode('utf-8').strip()
                print(f"Received: {line_str}")
                
                if line_str.startswith('DATA'):
                    # Parse the CSV data
                    # Expected format: DATA,timestamp,current,voltage,power,pf,freq
                    parts = line_str.split(',')
                    if len(parts) >= 7:  # Adjust based on actual data format
                        try:
                            timestamp = float(parts[1]) / 1000.0  # Convert ms to seconds
                            current = float(parts[2])
                            voltage = float(parts[3])
                            power = float(parts[4])
                            pf = float(parts[5])
                            freq = float(parts[6])
                            
                            # Update all data buffers
                            update_data_buffer(current_data, timestamp, current)
                            update_data_buffer(voltage_data, timestamp, voltage)
                            update_data_buffer(power_data, timestamp, power)
                            update_data_buffer(pf_data, timestamp, pf)
                            update_data_buffer(freq_data, timestamp, freq)
                            
                        except ValueError as e:
                            print(f"Error parsing values: {e}")
            except UnicodeDecodeError:
                print(f"Received binary data: {raw_line}")
    except Exception as e:
        print(f"Error reading serial port: {e}")
    
    # Update all plots
    if current_data.times:
        # Show last minute of data (60 seconds)
        current_time = current_data.times[-1]
        time_min = max(0, current_time - 60)
        
        for ax in [ax_current, ax_voltage, ax_power, ax_pf, ax_freq]:
            # Update x-axis limits
            ax.set_xlim(time_min, current_time + 1)
            # Format x-axis ticks
            ax.xaxis.set_major_locator(plt.MaxNLocator(6))
            ax.xaxis.set_major_formatter(plt.FormatStrFormatter('%.1f'))
        
        update_plot(ax_current, line_current, text_current, current_data, "Current")
        update_plot(ax_voltage, line_voltage, text_voltage, voltage_data, "Voltage")
        update_plot(ax_power, line_power, text_power, power_data, "Power")
        update_plot(ax_pf, line_pf, text_pf, pf_data, "PF")
        update_plot(ax_freq, line_freq, text_freq, freq_data, "Freq")
    
    return line_current, line_voltage, line_power, line_pf, line_freq

# Open the serial port
try:
    # First check if we need to close any existing connections to the port
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    
    # Print all available ports for debugging
    print("Available ports:")
    for port in ports:
        print(f" - {port.device} ({port.description})")
    
    # Configure serial port with DTR and RTS disabled from the start
    ser = serial.Serial(
        port=args.port,
        baudrate=args.baudrate,
        timeout=1,
        dsrdtr=False,   # Disable DSR/DTR flow control
        rtscts=False    # Disable RTS/CTS flow control
    )
    
    # Add a small delay to let the ESP32 stabilize
    time.sleep(0.5)
    
    # Disable DTR and RTS after opening to prevent auto-reset
    ser.dtr = False
    ser.rts = False
    
    # Add another small delay after changing DTR/RTS
    time.sleep(0.5)
    
    # Flush any existing data
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    
    print(f"Connected to {args.port} at {args.baudrate} baud")
    print("DTR and RTS signals disabled to prevent auto-reset")

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

# Adjust subplot layout with better spacing
plt.subplots_adjust(hspace=0.3)

# Start the animation with 1-second update interval (1000ms)
ani = FuncAnimation(fig, update, init_func=init, interval=args.interval, blit=True)

try:
    plt.show()
except KeyboardInterrupt:
    print("Exiting...")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.") 