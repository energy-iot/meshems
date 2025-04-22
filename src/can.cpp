 /*****************************************************************************
 * @file can.cpp
 * @brief Interface code to allow reading/writing of CAN 2.0 messages to an MCP2515 device
 * 
 * This module provides both transmit and receive functionality for CAN communications
 * using the MCP2515 CAN controller. Features include:
 *   - Configurable message transmission timing
 *   - Detailed debugging and error reporting
 *   - Support for both standard (11-bit) and extended (29-bit) CAN IDs
 * 
 * Hardware: ESP32 with MCP2515 CAN controller connected via SPI
 * Dependencies: mcp_can.h library
 * 
 * Author(s): Doug Mendonca, Liam O'Brien
 *****************************************************************************/
 
 #include <mcp_can.h>
 #include <SPI.h>
 #include <pins.h>

// Operating mode selection
enum CAN_OPERATION_MODE {
  CAN_MODE_READ_ONLY,    // Only receives messages
  CAN_MODE_WRITE_ONLY,   // Only sends messages
  CAN_MODE_READ_WRITE    // Both sends and receives messages
};

// Set operating mode here
#define CAN_OPERATING_MODE CAN_MODE_READ_ONLY
// Message transmission interval (milliseconds)
#define CAN_TX_INTERVAL 1000   // 1 second between transmitted messages
// Debug level
#define CAN_DEBUG_LEVEL 1      // 0=Off, 1=Basic, 2=Detailed
// Crystal frequency of your MCP2515 in this case 8MHZ for our dev board (Typical values: 8MHz, 16MHz)
#define MCP_CRYSTAL_FREQ MCP_8MHZ
// CAN bus data baud rate, in this case 500kbps (Typical values: 125kbps, 250kbps, 500kbps, 1Mbps)
#define MCP_BUS_BAUD CAN_500KBPS

// ==================== CAN Variables ====================

// Transmission timing
unsigned long prevTX = 0;                // Last transmission timestamp

// Transmit data buffer - customize this for your application
byte txData[8] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};

// Receive buffer
unsigned char rxLen = 0;                 // Length of received data
unsigned char rxBuf[8];                  // Buffer for received data
long unsigned int rxCanID;               // Received CAN message ID

// Serial output formatting
char msgString[128];                     // Buffer for debug messages

// ==================== SPI & CAN Controller Setup ====================

// Create custom SPI instance for CAN controller (ESP32 default SPI may be used by display)
SPIClass canSPI = SPIClass();

// Initialize CAN controller with the custom SPI instance
MCP_CAN CAN0(&canSPI, CAN0_CS);

// ==================== Function Declarations ====================

void setup_can();
void loop_can();
bool sendCANMessage(long unsigned int canId, byte length, byte *data);
void processReceivedMessage(long unsigned int canId, byte length, byte *data);
void printCANMessage(long unsigned int canId, byte length, byte *data, bool isTx);
 
void setup_can()
{   
  // Configure pin for interrupt signal from MCP2515
  pinMode(CAN0_INT, INPUT);

  // Initialize secondary SPI interface
  canSPI.begin(CAN0_SCK, CAN0_SO, CAN0_SI);

  if (CAN_DEBUG_LEVEL > 0) {
    Serial.println("INFO - Initializing CAN interface...");
  }

  // Initialize MCP2515 with selected baud rate and crystal frequency
  if (CAN0.begin(MCP_ANY, MCP_BUS_BAUD, MCP_CRYSTAL_FREQ) == CAN_OK) {
    if (CAN_DEBUG_LEVEL > 0) {
      Serial.println("INFO - MCP2515 Initialized Successfully!");
    }
  } else {
    Serial.println("ERROR: Failed to initialize MCP2515!");
    return;
  }

  // Set normal mode to allow messages to be transmitted and received
  CAN0.setMode(MCP_NORMAL);

  if (CAN_DEBUG_LEVEL > 0) {
    Serial.println("INFO - CAN interface is active");
    
    if (CAN_OPERATING_MODE == CAN_MODE_READ_ONLY) {
      Serial.println("INFO - CAN Mode: READ ONLY");
    } else if (CAN_OPERATING_MODE == CAN_MODE_WRITE_ONLY) {
      Serial.println("INFO - CAN Mode: WRITE ONLY");
    } else {
      Serial.println("INFO - CAN Mode: READ & WRITE");
    }
  }
}
 
void loop_can()
{
  // ---- RECEIVE SECTION ----
  if (CAN_OPERATING_MODE == CAN_MODE_READ_ONLY || CAN_OPERATING_MODE == CAN_MODE_READ_WRITE) {
    // Check if data is available
    if (CAN_MSGAVAIL == CAN0.checkReceive()) {
      // Read the message data
      CAN0.readMsgBuf(&rxCanID, &rxLen, rxBuf);
      
      // Process the received message
      processReceivedMessage(rxCanID, rxLen, rxBuf);
    }
  }
  
  // ---- TRANSMIT SECTION ----
  if (CAN_OPERATING_MODE == CAN_MODE_WRITE_ONLY || CAN_OPERATING_MODE == CAN_MODE_READ_WRITE) {
    // Check if it's time to send a message
    if (millis() - prevTX >= CAN_TX_INTERVAL) {
      prevTX = millis();
      
      // EXAMPLE: Send data with ID 0x100
      if (!sendCANMessage(0x100, 8, txData)) {
        if (CAN_DEBUG_LEVEL > 0) {
          Serial.println("ERROR: Failed to send CAN message");
        }
      }
      
      // For testing, increment the first byte of the data for the next transmission
      txData[0]++;
    }
  }
}


 /**
 * Send a CAN message with the specified ID and data
 * 
 * @param canId The CAN identifier (11-bit or 29-bit)
 * @param length The length of data (0-8 bytes)
 * @param data Pointer to the data array
 * @return true if successful, false otherwise
 */
bool sendCANMessage(long unsigned int canId, byte length, byte *data) {
  if (length > 8) {
    if (CAN_DEBUG_LEVEL > 0) {
      Serial.println("ERROR: CAN data length exceeds 8 bytes");
    }
    return false;
  }
  
  // Send the message
  byte sndStat = CAN0.sendMsgBuf(canId, 0, length, data);
  
  if (sndStat == CAN_OK) {
    if (CAN_DEBUG_LEVEL > 0) {
      printCANMessage(canId, length, data, true);
    }
    return true;
  } else {
    return false;
  }
}

/**
 * Process a received CAN message
 * 
 * @param canId The CAN identifier of the received message
 * @param length The length of received data
 * @param data Pointer to the received data array
 */
void processReceivedMessage(long unsigned int canId, byte length, byte *data) {
  // Print message details
  if (CAN_DEBUG_LEVEL > 0) {
    printCANMessage(canId, length, data, false);
  }
  
  // Add your custom message processing logic here
  // Example: Check for specific message IDs and process accordingly
  
  // if (canId == 0x123) {
  //   // Handle message with ID 0x123
  // } else if (canId == 0x456) {
  //   // Handle message with ID 0x456
  // }
}

/**
 * Print CAN message details to Serial
 * 
 * @param canId The CAN identifier
 * @param length The length of data
 * @param data Pointer to the data array
 * @param isTx True if this is a transmitted message, false if received
 */
void printCANMessage(long unsigned int canId, byte length, byte *data, bool isTx) {
  Serial.print(isTx ? "TX " : "RX ");
  
  // Print ID information
  if ((canId & 0x80000000) == 0x80000000) {
    // Extended ID (29-bit)
    Serial.print("Extended ID: 0x");
    Serial.print(canId & 0x1FFFFFFF, HEX);
  } else {
    // Standard ID (11-bit)
    Serial.print("Standard ID: 0x");
    Serial.print(canId, HEX);
  }
  
  // Print data length
  Serial.print("  Length: ");
  Serial.print(length);
  
  // Print data bytes
  Serial.print("  Data: ");
  for (byte i = 0; i < length; i++) {
    Serial.print("0x");
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    if (i < length - 1) Serial.print(", ");
  }
  
  Serial.println();
}
 
 /*********************************************************************************************************
   END FILE
 *********************************************************************************************************/
 