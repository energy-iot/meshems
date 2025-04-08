/* CAN Loopback Example
 * This example sends a message once a second and receives that message
 *   no CAN bus is required.  This example will test the functionality 
 *   of the protocol controller, and connections to it.
 *   
 *   Written By: Cory J. Fowler - October 5th 2016
 */

 #include <mcp_can.h>
 #include <SPI.h>
 #include <pins.h>

 // CAN TX Variables
 unsigned long prevTX = 0;                                        // Variable to store last execution time
 const unsigned int invlTX = 1000;                                // Five second interval constant
 byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};  // Generic CAN data to send
 
 // CAN RX Variables
//  long unsigned int rxId;
//  unsigned char len;
//  unsigned char rxBuf[8];
//  unsigned long canId;

unsigned char len = 0;    // length of received buffer
unsigned char buf[8];     // Buffer to hold up to 8 bytes of data
long unsigned int canID;       // Can message ID
 
 // Serial Output String Buffer
 char msgString[128];

 // Create a custom SPI instance
 SPIClass canSPI = SPIClass();
 // Create the CAN controller with custom SPI
 MCP_CAN CAN0(&canSPI, CAN0_CS);
 
 void setup_can()
 {   
  pinMode(CAN0_INT, INPUT);                           // Configuring pin for /INT input

  // Initialize your custom SPI
  canSPI.begin(CAN0_SCK, CAN0_SO, CAN0_SI);
  Serial.println("canSPI Initialized Successfully!");
  
  // Initialize MCP2515 and save the status code
  byte status = CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  
  // Print the status code in different formats
  Serial.print("MCP2515 Initialization Status Code: ");
  Serial.print(status);                  // Decimal

  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(status == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
    // Set to normal mode
    CAN0.setMode(MCP_NORMAL);
    Serial.println("MCP2515 in Normal Mode!");
  } else {
    Serial.println("Error Initializing MCP2515...");
  }
 
   CAN0.setMode(MCP_NORMAL);
   Serial.println("MCP2515 Example...");
 }
 
 void loop_can()
 {
  if(CAN_MSGAVAIL == CAN0.checkReceive())    //check if data is coming in
  {
      CAN0.readMsgBuf(&canID, &len, buf);    // Read data,  len: data length, buf: data buffer

      Serial.print("CAN ID: ");
      Serial.print(canID, HEX);     // print the CAN ID in HEX

      Serial.print("    Data Length: "); // Print the length of the received data
      Serial.print(len);
      Serial.print("    ");
      
      for(int i = 0; i<len; i++)    //loop on the incoming data to print each byte
      {
          Serial.print(buf[i]);     
          if(i<len-1) Serial.print(",");  // Separate the numbers for readability
      }
      Serial.println();
  }
  //  if(!digitalRead(CAN0_INT))                          // If CAN0_INT pin is low, read receive buffer
  //  {
  //    CAN0.readMsgBuf(&rxId, &len, rxBuf);              // Read data: len = data length, buf = data byte(s)
     
  //    if((rxId & 0x80000000) == 0x80000000)             // Determine if ID is standard (11 bits) or extended (29 bits)
  //      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
  //    else
  //      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
   
  //    Serial.print(msgString);
   
  //    if((rxId & 0x40000000) == 0x40000000){            // Determine if message is a remote request frame.
  //      sprintf(msgString, " REMOTE REQUEST FRAME");
  //      Serial.print(msgString);
  //    } else {
  //      for(byte i = 0; i<len; i++){
  //        sprintf(msgString, " 0x%.2X", rxBuf[i]);
  //        Serial.print(msgString);
  //      }
  //    }
         
  //    Serial.println();
  // }
  
  // UNCOMMENT IF SENDER
  //  if(millis() - prevTX >= invlTX){                    // Send this at a N second interval. 
  //    prevTX = millis();
  //    byte sndStat = CAN0.sendMsgBuf(0x100, 8, data);
     
  //    if(sndStat == CAN_OK) {
  //      Serial.println("CAN Message Sent Success!"); // TODO perhaps display data or more meaningful, iterate a CAN sent success stats metric
  //      Serial.println(sndStat);
  //    }
  //    else {
  //      Serial.println("CAN Error Sending Message"); // TODO perhaps display data or more meaningful, iterate a CAN sent error stats metric
  //    }
  //  }
 }
 
 /*********************************************************************************************************
   END FILE
 *********************************************************************************************************/
 