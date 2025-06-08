#include <SPI.h>
#include <mcp2515.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x08

struct can_frame canMsgRequest;
struct can_frame canMsgResponse;
MCP2515 mcp2515(10);

char mileageMessage[10];
char speedMessage[10];
char fuelMessage[10];
int mileage;
int speed;
int fuel;
char charArray[50]= "hello";

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);

  while (!Serial);

  // Initialize the CAN bus
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  // Prepare the OBD-II mileage request message
  canMsgRequest.can_id  = 0x7DF;   // Broadcast ID
  canMsgRequest.can_dlc = 8;       // Data length
  canMsgRequest.data[0] = 0x02;    // Number of additional bytes
  canMsgRequest.data[1] = 0x01;    // Mode 01
  canMsgRequest.data[2] = 0x31;    // PID for mileage
  canMsgRequest.data[3] = 0x00;    // Unused
  canMsgRequest.data[4] = 0x00;    // Unused
  canMsgRequest.data[5] = 0x00;    // Unused
  canMsgRequest.data[6] = 0x00;    // Unused
  canMsgRequest.data[7] = 0x00;    // Unused

  Serial.println("CAN initialized and ready to send messages");
}

void loop() {
  readMileage();
  delay(500);
  readSpeed();
  delay(500);  // Delay between requests
  readFuel();
  delay(500);  // Delay between requests
  String Data = "";
  Data += "M: " + String(mileageMessage) + ", S: " + String(speedMessage) + ", F: " + String(fuelMessage);
  

// Copy string to char array
  Data.toCharArray(charArray, 32);

// Now charArray contains the characters from str

  Serial.println(charArray);
}

void readMileage() {
  // Send the mileage request
  canMsgRequest.data[2] = 0x31;  // PID for mileage

  if (mcp2515.sendMessage(&canMsgRequest) != MCP2515::ERROR_OK) {
    Serial.println("Error sending mileage request");
    delay(1000);
    return;
  }

  // Wait for a response
  unsigned long startTime = millis();
  bool mileageReceived = false;

  while ((millis() - startTime) < 500) {  // Wait for up to 0.5 seconds for a response
    if (mcp2515.readMessage(&canMsgResponse) == MCP2515::ERROR_OK) {  // Check if data is available
      if (canMsgResponse.can_id == 0x7E8 && canMsgResponse.data[2] == 0x31) {
        // Extract mileage from the response
        mileage = canMsgResponse.data[3] * 256 + canMsgResponse.data[4];
        Serial.print("Mileage: ");
        Serial.println(mileage);
        ltoa(mileage, mileageMessage, 10); // use ltoa to handle larger numbers
        break;
      }
    }
  }
}

void readSpeed() {
  // Send the speed request
  canMsgRequest.data[2] = 0x0D;  // PID for speed

  if (mcp2515.sendMessage(&canMsgRequest) != MCP2515::ERROR_OK) {
    Serial.println("Error sending speed request");
    delay(1000);
    return;
  }

  // Wait for a response
  unsigned long startTime = millis();
  bool speedReceived = false;

  while ((millis() - startTime) < 500) {  // Wait for up to 0.5 seconds for a response
    if (mcp2515.readMessage(&canMsgResponse) == MCP2515::ERROR_OK) {  // Check if data is available
      if (canMsgResponse.can_id == 0x7E8 && canMsgResponse.data[2] == 0x0D) {
        // Extract speed from the response
        speed = canMsgResponse.data[3];
        Serial.print("Speed: ");
        Serial.println(speed);
        ltoa(speed, speedMessage, 10); // use ltoa to handle larger numbers
        break;
      }
    }
  }
}

void readFuel() {
  // Send the speed request
  canMsgRequest.data[2] = 0x2F;  // PID for fuel

  if (mcp2515.sendMessage(&canMsgRequest) != MCP2515::ERROR_OK) {
    Serial.println("Error sending fuel request");
    delay(1000);
    return;
  }

  // Wait for a response
  unsigned long startTime = millis();
  bool fuelReceived = false;

  while ((millis() - startTime) < 500) {  // Wait for up to 0.5 seconds for a response
    if (mcp2515.readMessage(&canMsgResponse) == MCP2515::ERROR_OK) {  // Check if data is available
      if (canMsgResponse.can_id == 0x7E8 && canMsgResponse.data[2] == 0x2F) {
        // Extract fuel from the response
        fuel = canMsgResponse.data[3];
        Serial.print("Fuel: ");
        Serial.println(fuel);
        ltoa(fuel, fuelMessage, 10); // use ltoa to handle larger numbers
        break;
      }
    }
  }
}

void requestEvent() {
  // Send mileage and speed messages to the master
  Wire.write(charArray);
  
}
