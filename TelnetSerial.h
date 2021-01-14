/*
 * Telnet Serial library
 * 
 * This library will move the standard Serial function to use a Telnet Connection.
 * 
 * There are two significant side effects of this Library:
 * 1) Since Serial.swap() is used to make this function work 2 additional pins 
 *    are consumed and not available to control devices.
 *    Pins GPIO15 and GPIO13 are used to intercept the data and send/receive
 *    via Telnet.
 * 2) 
 */
#pragma once

#ifndef __TELNETSERIAL_H
#define __TELNETSERIAL_H

#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

//
// Definitions for sending Serial print statements to Telnet Clients
//
#define TELNETSERIAL_STACK_PROTECTOR  512 // bytes

//how many clients should be able to telnet to this ESP8266
#define TELNETSERIAL_DEFAULT_PORT 23
#define TELNETSERIAL_DEFAULT_BAUD 115200
#define TELNETSERIAL_DEFAULT_RXBUFFERSIZE 1024

class TelnetSerial {

  public:
    TelnetSerial();

    // Telnet Serial functions
    bool begin() { return begin(TELNETSERIAL_DEFAULT_BAUD); }
    bool begin(unsigned long baud);
    void setPort(unsigned long port);
    void setRxBufferSize(unsigned long port);
    void handle();

    // USB/Hardware Serial functions
    Stream *getOriginalSerial();

  protected:
    // USB/Hardware Serial functions
    bool beginOriginalSerial(unsigned long baud);

  private:
  
    //Telnet Serial variables
    unsigned long _port = TELNETSERIAL_DEFAULT_PORT;
    WiFiServer _telnetServer = WiFiServer(_port);
    WiFiClient _telnetServerClient;

    // USB/Hardware Serial variables
    unsigned long _baud = TELNETSERIAL_DEFAULT_BAUD;
    unsigned long _rxbuffersize = TELNETSERIAL_DEFAULT_RXBUFFERSIZE;
    SoftwareSerial* originalSerial = nullptr;

};

#endif  // __TELNETSERIAL_H
