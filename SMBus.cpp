/*

*/

#include "SMBus.h"
#include <Wire.h>



/// @brief 
/// @param handler Pointer to a member function to accept a buffer of the received message
/// @param instance generic pointer to the instance of the class, the function is member of. Must be casted.
SystemBus::iHandler::iHandler(iReceiveHandler* handler, void* instance) {
    Handler = handler;
    Instance = instance;
}
SystemBus::iHandler::iHandler() {}

void SystemBus::HandlerQueue(int length) {
    char buf[length];
    byte received = Wire.readBytes(buf, length);
    if (received < length) {
        Serial.println("Unexpected transmission end in message from device!");
    }

    for (byte i = 0; i <= HandlerCount; i++) {
        if (Handlers[i].Handler->ReceiveHandler(buf, length)) {
            Serial.println("[Dbg] (" + String((byte) Handlers[i].Handler, HEX) + ") calling receive handler at address \"" + String((byte) Handlers[i].Handler, HEX) + "\":");
        }
    }
}
typedef SystemBus::ReturnCode ReturnCode;

void SystemBus::begin() const {
    Wire.begin();
}

/// @brief Initialize as slave
/// @param address Address of self
void SystemBus::begin(byte address) const {
    Wire.begin(address);
}

unsigned int SystemBus::fetchWord(byte address, byte func) const {
    Wire.beginTransmission(address << 1);
    Wire.write(func);
    Wire.requestFrom((address << 1), 0x2, true);
    byte word[func];
    word[0] = Wire.read();
    word[1] = Wire.read();  // Receive a byte as character
    return (word[0] | (word[1] << 8));
}

ReturnCode SystemBus::readBlock(byte address, byte func, byte* blockBuffer, byte* len) const {
    Wire.beginTransmission(address << 1);
    Wire.write(func);
    Wire.requestFrom(static_cast<byte>(address << 1), *len, static_cast<byte>(true));
    len = 0;
    while (Wire.available()) {
        blockBuffer[*len] = Wire.read();  // Receive a byte as character
        *len++;
    }

    return (ReturnCode) Wire.endTransmission(true);
}

void SystemBus::addReceiveHandler(iHandler Handler) {
    Wire.onReceive(HandlerQueue);

    Handlers[HandlerCount] = Handler;
    HandlerCount++;
}

void SystemBus::scan() const {

    for (byte i = 0; i < 16; i++) {
        Serial.print(" ");
        Serial.print(i, HEX);
    }
    Serial.println("         ");

    for (byte address = 0x03; address < 127; ++address) {
        // The i2c_scanner uses the return value of
        // the Wire.endTransmission to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        ReturnCode error = (ReturnCode) Wire.endTransmission();

        switch (error) {
            case Success:
                Serial.print(" 0x");
                if (address < 16) {
                    Serial.print("0");
                }
                Serial.print(address, HEX);
                break;
            case ERR:
                Serial.print(" XX");
                break;
            case Timeout:
                Serial.print(" --");
                break;
            default:
                Serial.print(" NA");
                break;
        }
        if (address % 15 == 0) {
            Serial.println();
        }
    }
    Serial.println();
}
SystemBus::iHandler SystemBus::Handlers[8] = {}; // Define and initialize the static vars
byte SystemBus::HandlerCount = 0;

// Preinstantiate Objects //////////////////////////////////////////////////////

SystemBus SMBus = SystemBus();
