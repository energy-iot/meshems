/**
 * CAN Bus Interface Header
 * 
 * Provides declarations for CAN bus functionality with both read and write support.
 */

 #pragma once

 #include <mcp_can.h>
 #include <SPI.h>
 
 // Operating mode selection
 enum CAN_OPERATION_MODE {
   CAN_MODE_READ_ONLY,    // Only receives messages
   CAN_MODE_WRITE_ONLY,   // Only sends messages
   CAN_MODE_READ_WRITE    // Both sends and receives messages
 };
 
 // Function declarations
 void setup_can();
 void loop_can();
 bool sendCANMessage(long unsigned int canId, byte length, byte *data);
 void setCANOperatingMode(CAN_OPERATION_MODE mode);
 CAN_OPERATION_MODE getCANOperatingMode();
 
 // Optional callback registration
 void registerCANReceiveCallback(void (*callback)(long unsigned int, byte, byte*));