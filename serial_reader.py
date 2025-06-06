#!/usr/bin/env python3

import serial
import serial.tools.list_ports
import argparse
import datetime
import sys
from typing import Optional, List

class SerialReader:
    def __init__(self, port: str, baudrate: int, timeout: float):
        """Initialize the serial reader with the given parameters."""
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial_connection: Optional[serial.Serial] = None

    @staticmethod
    def list_available_ports() -> List[str]:
        """List all available serial ports."""
        return [port.device for port in serial.tools.list_ports.comports()]

    @staticmethod
    def find_usb_modem() -> Optional[str]:
        """Find the first available USB modem device."""
        for port in serial.tools.list_ports.comports():
            if 'usbmodem' in port.device.lower():
                return port.device
        return None

    def connect(self) -> bool:
        """Establish connection to the serial port."""
        try:
            self.serial_connection = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout
            )
            print(f"Successfully connected to {self.port}")
            return True
        except serial.SerialException as e:
            print(f"Error connecting to {self.port}: {str(e)}", file=sys.stderr)
            available_ports = self.list_available_ports()
            if available_ports:
                print("\nAvailable ports:", file=sys.stderr)
                for port in available_ports:
                    print(f"  - {port}", file=sys.stderr)
            else:
                print("\nNo serial ports found. Please check your connections.", file=sys.stderr)
            return False

    def read_line(self) -> Optional[str]:
        """Read a single line from the serial port."""
        if not self.serial_connection:
            return None
        
        try:
            if self.serial_connection.in_waiting:
                return self.serial_connection.readline().decode('utf-8').strip()
        except serial.SerialException as e:
            print(f"Error reading from serial port: {str(e)}", file=sys.stderr)
        except UnicodeDecodeError as e:
            print(f"Error decoding data: {str(e)}", file=sys.stderr)
        
        return None

    def continuous_read(self):
        """Continuously read data from the serial port."""
        if not self.connect():
            return

        print("Starting continuous read. Press Ctrl+C to stop.")
        try:
            while True:
                line = self.read_line()
                if line:
                    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
                    print(f"[{timestamp}] {line}")
        except KeyboardInterrupt:
            print("\nStopping serial reader...")
        finally:
            if self.serial_connection:
                self.serial_connection.close()
                print("Serial connection closed.")

def main():
    # Try to find a USB modem device first
    default_port = SerialReader.find_usb_modem()
    if not default_port:
        # Fall back to first available port if no USB modem is found
        available_ports = SerialReader.list_available_ports()
        default_port = available_ports[0] if available_ports else ""

    parser = argparse.ArgumentParser(description="Serial Port Reader")
    parser.add_argument("--port", default=default_port, help="Serial port to connect to")
    parser.add_argument("--baudrate", type=int, default=9600, help="Baud rate")
    parser.add_argument("--timeout", type=float, default=1.0, help="Read timeout in seconds")
    parser.add_argument("--list-ports", action="store_true", help="List available serial ports and exit")
    
    args = parser.parse_args()

    if args.list_ports:
        print("Available serial ports:")
        for port in SerialReader.list_available_ports():
            print(f"  - {port}")
        return

    if not args.port:
        print("Error: No serial ports found. Please check your connections.", file=sys.stderr)
        sys.exit(1)

    reader = SerialReader(args.port, args.baudrate, args.timeout)
    reader.continuous_read()

if __name__ == "__main__":
    main() 