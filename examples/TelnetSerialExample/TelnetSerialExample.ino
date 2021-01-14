#include <ESP8266WiFi.h>
#include "TelnetSerial.h"

const char* ssid = "";
const char* password = "";


// Used for running a simple program during loop to blink led
// On board LED used to show status
#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif
const int ledPin =  LED_BUILTIN;  // the number of the LED pin
unsigned long previousMillis = 0;     // will store last time LED was updated
const long interval = 5000;           // interval at which to blink (milliseconds)
boolean ledState = LOW;               // Used for blinking LEDs


TelnetSerial telnetSerial;
Stream *usbSerial;

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  Serial.println(ESP.getFullVersion());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.print("connected, address=");
  Serial.println(WiFi.localIP());
  
  telnetSerial.begin(115200);
  usbSerial = telnetSerial.getOriginalSerial();

  usbSerial->print("Ready! Use 'telnet ");
  usbSerial->print(WiFi.localIP());
  usbSerial->printf(" %d' to connect\n", TELNETSERIAL_DEFAULT_PORT);

}

void loop() {
  telnetSerial.handle();

  
  // Blinking LED code to be replaced with real code
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      usbSerial->println("usbSerial HIGH");
      Serial.println("serial HIGH");
    } else {
      ledState = LOW;
      usbSerial->println("usbSerial LOW");
      Serial.println("serial LOW");
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }

}
