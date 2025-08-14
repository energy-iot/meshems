# EMS-Dev Python Gateway

Energy Management System Python implementation for inverter communication with SunSpec Modbus TCP server.

## Overview

This project provides a Python-based gateway for communicating with various inverter types over Modbus RTU and exposing the data via a SunSpec-compliant Modbus TCP server. The architecture supports multiple inverter types with different register mappings while maintaining backward compatibility with existing Sol-Ark installations.

## Features

- **Multi-Inverter Support**: Supports Sol-Ark and generic inverter types with configurable register mappings
- **SunSpec Compliance**: Exposes inverter data in standardized SunSpec format over Modbus TCP
- **Extensible Architecture**: Abstract base classes allow easy addition of new inverter types
- **Phase Support**: Proper handling of single-phase, split-phase, and three-phase inverters
- **Multiple Model Instances**: Support for multiple instances of SunSpec models (701, 713, 714)
- **Configurable Register Mappings**: JSON-based configuration for defining register mappings for different inverter types

## Architecture

The system is built on an extensible architecture with the following components:

1. **Abstract Base Classes**: Define interfaces for inverter communication, data structures, and register mappings
2. **Concrete Implementations**: Specific implementations for different inverter types (Sol-Ark, Generic, etc.)
3. **Inverter Factory**: Creates appropriate inverter clients based on configuration
4. **Register Mapping System**: Handles different register mappings for various inverter types
5. **SunSpec Models**: Implements SunSpec-compliant data models for exposing inverter data
6. **Modbus Server**: SunSpec-compliant Modbus TCP server for external access to inverter data

## Supported Inverter Types

### Sol-Ark Inverters

Native support for Sol-Ark inverters with predefined register mappings.

### Generic Inverters

Configurable support for other inverter types through JSON-based register mapping files.

## Configuration

The system is configured through `config.yaml` which supports:

- Serial port configuration
- Inverter type and communication settings
- SunSpec server configuration
- Device information for SunSpec models
- Monitoring and alert settings

## Installation

```bash
# Clone the repository
git clone https://github.com/energy-iot/ems-dev.git
cd ems-dev

# Install dependencies
pip install -r requirements.txt

# Run the application
python -m ems.main
```

## Usage

### Normal Operation

```bash
# Run with default configuration
python -m ems.main

# Run with custom configuration file
python -m ems.main --config my_config.yaml

# Enable verbose logging
python -m ems.main --verbose
```

### Test Mode

```bash
# Run in test mode (single poll and exit)
python -m ems.main --test
```

## SunSpec Model Support

The implementation supports multiple SunSpec models:

- **Model 1**: Common model for device identification
- **Model 701**: Inverter model (multiple instances for grid and load)
- **Model 713**: Battery model for storage capacity monitoring
- **Model 714**: DC measurement model with multiple ports

## Register Mapping Configuration

For generic inverters, register mappings are defined in JSON files with the following structure:

```json
{
  "inverter_type": "generic_example",
  "version": "1.0",
  "models": {
    "grid_model": {
      "sunspec_model": 701,
      "instance": 1,
      "context": "grid",
      "description": "Grid-side AC measurements",
      "data_points": {
        "ac_power": {"register": 169, "scaling": 1.0, "sunspec_point": "W"}
      }
    }
  }
}
```

## Development

### Adding New Inverter Types

To add support for a new inverter type:

1. Create a new implementation of `InverterClient` and `RegisterMapping`
2. Add the inverter type to `InverterFactory`
3. Create register mapping configuration files if needed

### Extending SunSpec Models

To add support for additional SunSpec models:

1. Create new data classes for the model
2. Add register mapping definitions
3. Update the SunSpec mapper to handle the new model

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
