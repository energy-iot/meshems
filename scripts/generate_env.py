#!/usr/bin/env python3
"""
Generate a C++ header file from a .env file
"""

import os
import sys

def main():
    # Check if .env file exists
    if not os.path.exists('.env'):
        print("Error: .env file not found")
        sys.exit(1)
    
    # Read .env file
    with open('.env', 'r') as f:
        env_lines = f.readlines()
    
    # Generate header file content
    header_content = """// Auto-generated file from .env
// Do not edit directly!

#pragma once

"""
    
    # Process each line in .env file
    for line in env_lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        
        key, value = line.split('=', 1)
        
        # Handle different types of values
        if key == 'MQTT_PORT' or key == 'ENABLE_MQTT':
            # Integer values
            header_content += f"#define {key} {value}\n"
        else:
            # String values
            header_content += f"#define {key} \"{value}\"\n"
    
    # Write header file
    with open('include/env_config.h', 'w') as f:
        f.write(header_content)
    
    print("Generated include/env_config.h")

if __name__ == "__main__":
    main()
