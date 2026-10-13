#include <Arduino.h>
#include <string.h>
#include "Keypad4x4.h"
#include "OpticalTx.h"

const uint8_t STX = 0x02;
const uint8_t ETX = 0x03;
const uint8_t MAX_MESSAGE = 16;

char message[MAX_MESSAGE + 1] = {};
char previousMessage[MAX_MESSAGE + 1] = {};
uint8_t messageLength = 0;
uint8_t previousLength = 0;

void transmit(const char *text, uint8_t length) {
  OpticalTx::sendByte(STX);
  for (uint8_t i = 0; i < length; i++) {
    OpticalTx::sendByte((uint8_t)text[i]);
  }
  OpticalTx::sendByte(ETX);
}

void sendMessage() {
  if (messageLength > 0) {
    previousLength = messageLength;
    memcpy(previousMessage, message, messageLength + 1);
  }

  if (previousLength == 0) return;

  Serial.print(F("TX: "));
  Serial.println(previousMessage);
  Serial.flush();

  transmit(previousMessage, previousLength);

  messageLength = 0;
  message[0] = '\0';
  delay(150);  // receiver updates the OLED after ETX
}

void setup() {
  Serial.begin(115200);
  Keypad4x4::begin();
  OpticalTx::begin();
  Serial.println(F("Optical transmitter ready"));
}

void loop() {
  char key = Keypad4x4::read();
  if (!key) return;

  delay(25);
  if (Keypad4x4::read() != key) return;

  if (key == '*') {
    sendMessage();
  } else if (messageLength < MAX_MESSAGE) {
    // Two spare keypad keys are mapped to H and I for the demo.
    if (key == 'D') key = 'H';
    else if (key == '#') key = 'I';

    message[messageLength++] = key;
    message[messageLength] = '\0';
    Serial.print(F("Typing: "));
    Serial.println(message);
  }

  while (Keypad4x4::read()) delay(5);
  delay(20);
}
