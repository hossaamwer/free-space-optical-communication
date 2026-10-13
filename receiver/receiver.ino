#include <Arduino.h>
#include "MiniOLED.h"
#include "OpticalRx.h"

const uint8_t STX = 0x02;
const uint8_t ETX = 0x03;
const uint8_t MAX_MESSAGE = 16;

char message[MAX_MESSAGE + 1] = {};
uint8_t messageLength = 0;
bool receiving = false;
bool oledReady = false;
uint32_t lastByteTime = 0;

MiniOLED display(0x3C);

void showMessage() {
  if (oledReady) {
    display.clear();
    display.setCursor(0, 0);
    display.print("RECEIVED");
    display.setBigCursor(0, 24);
    display.printBig(message);
  }

  Serial.print(F("RX: "));
  Serial.println(message);
}

void processByte(uint8_t value) {
  if (value == STX) {
    receiving = true;
    messageLength = 0;
    message[0] = '\0';
    return;
  }

  if (value == ETX) {
    if (receiving && messageLength > 0) showMessage();
    receiving = false;
    return;
  }

  if (!receiving) return;

  if (messageLength >= MAX_MESSAGE || value < 32 || value > 126) {
    receiving = false;
    return;
  }

  message[messageLength++] = (char)value;
  message[messageLength] = '\0';
}

void setup() {
  Serial.begin(115200);
  OpticalRx::begin();

  oledReady = display.begin();
  if (oledReady) {
    display.setCursor(0, 0);
    display.print("OPTICAL RX");
    display.setBigCursor(15, 24);
    display.printBig("WAITING");
  }

  Serial.println(F("Optical receiver ready"));
}

void loop() {
  uint8_t value;
  if (OpticalRx::poll(value)) {
    lastByteTime = millis();
    processByte(value);
  }

  if (receiving && (uint32_t)(millis() - lastByteTime) > 30) {
    receiving = false;
  }
}
