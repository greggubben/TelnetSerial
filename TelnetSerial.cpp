/*
 * Telnet Serial library
 * 
 * This Library will move Serial read & write from USB to Telnet.
 * 
 */
#include "TelnetSerial.h"
#include <algorithm>        // std::min

#include <esp8266_peri.h>   // Access to Peripheral Registers


/*
 * Telnet Serial Constructor
 * Sets up defaults
 */
TelnetSerial::TelnetSerial(void) {
  _baud = TELNETSERIAL_DEFAULT_BAUD;
  _port = TELNETSERIAL_DEFAULT_PORT;
  _rxbuffersize = TELNETSERIAL_DEFAULT_RXBUFFERSIZE;
}


/******************************
 * Telnet Serial Functions
 ******************************/

void TelnetSerial::setPort(unsigned long port) {
  _port = port;
}

void TelnetSerial::setRxBufferSize(unsigned long rxBufferSize) {
  _rxbuffersize = rxBufferSize;
}

/*
 * Start the Telnet Serial service with specified baud rate
 */
bool TelnetSerial::begin(unsigned long baud) {
  _baud = baud;
  
  if ((WiFi.status() == WL_CONNECTED) && (Serial)) {
    Serial.flush();
    Serial.setRxBufferSize(_rxbuffersize);
    Serial.swap();

    // start backup USB/Hardware Serial
    beginOriginalSerial(baud);

    //start telnet server
    _telnetServer.begin(_port);
    _telnetServer.setNoDelay(true);
    return true;
  }
  else {
    return false;
  }

}

void TelnetSerial::handle() {
  //check if there are any new telnet clients
  if (_telnetServer.hasClient()) {

    if (!_telnetServerClient.connected()) { 
      // Accept new telnet client
      _telnetServerClient = _telnetServer.available();
      originalSerial->println("New client");
    }
    else {
      // Already have a telnet client - reject additional client
      _telnetServer.available().println("busy");
      // hints: server.available() is a WiFiClient with short-term scope
      // when out of scope, a WiFiClient will
      // - flush() - all data will be sent
      // - stop() - automatically too
      originalSerial->println("Server is busy with an active connection.");
    }
  }

  // check if telnet client sent data to recieve
  while (_telnetServerClient.available() && Serial.availableForWrite() > 0) {
    // working char by char is not very efficient
    Serial.write(_telnetServerClient.read());
  }

  // Determine max amount of data the Telnet Client can receive at a time
  size_t maxBytesToSendTelnetClient = 0;
  if (_telnetServerClient) {
    // client.availableForWrite() returns 0 when !client.connected()
    size_t telnetClientWriteBufferSize = _telnetServerClient.availableForWrite();
    if (telnetClientWriteBufferSize) {
      maxBytesToSendTelnetClient = telnetClientWriteBufferSize;
    }
    else {
      // warn but ignore congested clients
      originalSerial->println("Client is congested.");
    }
  }

  // Sent any data out to telnet client
  size_t dataToSendLen = std::min((size_t)Serial.available(), maxBytesToSendTelnetClient);
  dataToSendLen = std::min(dataToSendLen, (size_t)TELNETSERIAL_STACK_PROTECTOR);
  if (dataToSendLen) {
    uint8_t dataToSend[dataToSendLen];
    size_t dataReadFromSerialLen = Serial.readBytes(dataToSend, dataToSendLen);
    // push UART data to connected telnet client
    // if client.availableForWrite() was 0 (congested)
    // and increased since then,
    // ensure write space is sufficient:
    if (_telnetServerClient.availableForWrite() >= dataReadFromSerialLen) {
      size_t dataSentToTelnetClientLen = _telnetServerClient.write(dataToSend, dataReadFromSerialLen);
      if (dataSentToTelnetClientLen != dataToSendLen) {
        originalSerial->printf("data length mismatch: to send:%zd readFromSerial:%zd sentToTelnetClient:%zd\n", dataToSendLen, dataReadFromSerialLen, dataSentToTelnetClientLen);
      }
    }
  }
}


/******************************
 * USB/Hardware Serial Functions
 ******************************/

bool TelnetSerial::beginOriginalSerial(unsigned long baud) {
  // use SoftwareSerial on regular RX(3)/TX(1) for logging
  originalSerial = new SoftwareSerial(3, 1);
  originalSerial->begin(baud);
  originalSerial->enableIntTx(false);
  originalSerial->println("\n\nUsing SoftwareSerial for logging");

  USC0(0) |= (1 << UCLBE); // incomplete HardwareSerial API
  originalSerial->println("Serial Internal Loopback enabled");
}

Stream *TelnetSerial::getOriginalSerial () {
  if (originalSerial) {
    return originalSerial;
  }
  else {
    return (&Serial);
  }
}
