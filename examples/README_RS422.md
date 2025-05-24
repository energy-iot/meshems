# RS422 Basic Example

This directory contains a basic example implementation for RS422 serial communication using a QYF-998 RS422 to TTL serial module with an ESP32.

## Overview

The RS422 basic example demonstrates bidirectional serial communication over RS422 protocol, including:

- **Message Reception**: Receives and processes incoming RS422 messages
- **Command Processing**: Responds to simple commands (PING, STATUS, ECHO)
- **Periodic Messaging**: Sends heartbeat and status messages automatically
- **Display Integration**: Shows messages on both serial monitor and OLED display
- **Statistics Tracking**: Keeps count of sent and received messages

## Hardware Requirements

### Components
- ESP32 development board (ESP32-S3 recommended)
- QYF-998 RS422 to TTL serial converter module
- RS422 device or another QYF-998 module for testing
- Connecting wires
- Optional: OLED display (SH1106)

### Pin Connections

#### QYF-998 Module to ESP32
```
QYF-998 Pin    ESP32 Pin        Description
-----------    ---------        -----------
VCC            3.3V or 5V       Power supply (check module specs)
GND            GND              Ground
RXD            RS422_RX_1       Serial receive (defined in pins.h)
TXD            RS422_TX_1       Serial transmit (defined in pins.h)
```

#### QYF-998 Module to RS422 Device
```
QYF-998 Pin    RS422 Device     Description
-----------    ------------     -----------
TX+            RX+              Differential transmit positive
TX-            RX-              Differential transmit negative
RX+            TX+              Differential receive positive
RX-            TX-              Differential receive negative
```

## Software Setup

### Pin Configuration
The RS422 pins are defined in `include/pins.h`:
```cpp
#define RS422_RX_1     47   // RX pin for QYF-998 module
#define RS422_TX_1     21   // TX pin for QYF-998 module
```

### Baud Rate
The example uses 9600 baud rate, which is reliable for SoftwareSerial on ESP32:
```cpp
const int RS422_BAUD_RATE = 9600;
```

## Usage

### Basic Integration
To use the RS422 module in your project:

1. Include the header file:
```cpp
#include <rs422.h>
```

2. Initialize in setup():
```cpp
void setup() {
    setup_rs422();
}
```

3. Process messages in loop():
```cpp
void loop() {
    loop_rs422();
}
```

### Available Commands

The example responds to these commands:

| Command | Response | Description |
|---------|----------|-------------|
| `PING` | `PONG` | Simple connectivity test |
| `STATUS` | `STATUS: RX=n TX=m` | Returns message statistics |
| `ECHO <message>` | `ECHO: <message>` | Echoes back the message |
| Any other | `UNKNOWN COMMAND: <message>` | Error response |

### Automatic Messages

The system automatically sends:
- **Heartbeat**: Every 10 seconds (`HEARTBEAT <uptime>s`)
- **Status**: Every 30 seconds (`AUTO_STATUS: RX=n TX=m Uptime=<time>s`)

### API Functions

#### Core Functions
```cpp
void setup_rs422();                           // Initialize RS422 communication
void loop_rs422();                            // Process messages and periodic tasks
```

#### Utility Functions
```cpp
void sendRS422CustomMessage(const String& message);  // Send custom message
void getRS422Stats(int& rxCount, int& txCount);      // Get message statistics
```

## Testing

### Loopback Test
For basic testing, you can connect TX+ to RX+ and TX- to RX- on the same QYF-998 module to create a loopback. Messages sent will be received back.

### Two-Device Test
Connect two QYF-998 modules:
```
Device A TX+ ←→ Device B RX+
Device A TX- ←→ Device B RX-
Device A RX+ ←→ Device B TX+
Device A RX- ←→ Device B TX-
```

### Expected Serial Output
```
RS422 Basic Example Starting...
INFO - RS422 Basic Example initialized
INFO - RS422 Baud Rate: 9600
INFO - RS422 RX Pin: 47
INFO - RS422 TX Pin: 21
INFO - RS422 Commands: PING, STATUS, ECHO <message>
Sent: RS422 Basic Example Started
RS422 Basic Example Ready!
Try sending these commands via RS422:
  PING
  STATUS
  ECHO Hello World
RS422 Sent: HEARTBEAT 10s
RS422 Stats - RX: 0, TX: 2
```

## Troubleshooting

### No Messages Received
1. Check wiring connections
2. Verify baud rate matches on both devices
3. Ensure proper RS422 differential connections
4. Check power supply to QYF-998 module

### Garbled Messages
1. Verify baud rate settings
2. Check for electrical interference
3. Ensure proper grounding
4. Try lower baud rate (e.g., 4800)

### SoftwareSerial Issues
1. SoftwareSerial works best at lower baud rates on ESP32
2. Avoid using pins that conflict with other peripherals
3. Consider using HardwareSerial if available

## Customization

### Changing Baud Rate
Modify the baud rate in `src/rs422.cpp`:
```cpp
const int RS422_BAUD_RATE = 4800;  // or other supported rate
```

### Adding Custom Commands
Add new command processing in the `processRS422Command()` function:
```cpp
else if (cmd == "MYCOMMAND") {
    sendRS422Message("MY_RESPONSE");
    _console.addLine("Custom command");
}
```

### Adjusting Timing
Modify the timing intervals:
```cpp
const unsigned long HEARTBEAT_INTERVAL = 5000;   // 5 seconds
const unsigned long STATUS_INTERVAL = 15000;     // 15 seconds
```

## Files

- `src/rs422.cpp` - Main implementation
- `include/rs422.h` - Header file with function declarations
- `examples/rs422_basic_example.cpp` - Standalone example
- `examples/README_RS422.md` - This documentation

## License

This example is part of the EMS Dev Kit project and follows the same licensing terms.
