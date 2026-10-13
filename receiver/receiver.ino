// Optical receiver: classic 16 MHz ATmega328P Nano, inverted digital module.
#include <Arduino.h>
#include <avr/interrupt.h>
#include "MiniOLED.h"
#if !defined(__AVR_ATmega328P__) || F_CPU != 16000000UL
#error "Use a classic 16 MHz ATmega328P UNO/Nano"
#endif
const uint16_t BIT_TICKS = 100; // 50 us at Timer1 / 8.
const byte STX = 0x02, ETX = 0x03, MAX_MESSAGE = 16;
char message[MAX_MESSAGE + 1] = {};
byte messageLength = 0;
bool receiving = false, oledReady = false;
uint32_t lastByteTime = 0;
MiniOLED display(0x3C);

inline byte opticalBit() {
  return (PIND & _BV(PD2)) ? 0 : 1; // D2: module LOW means laser ON.
}
void waitUntil(uint16_t tick) {
  while ((int16_t)(TCNT1 - tick) < 0) {}
}
bool receiveByte(byte &value, uint16_t edgeTime) {
  byte savedSREG = SREG;
  cli();
  uint16_t sample = edgeTime + BIT_TICKS / 2;
  waitUntil(sample);
  if (opticalBit() != 0) { SREG = savedSREG; return false; }
  value = 0;
  for (byte i = 0; i < 8; i++) {
    sample += BIT_TICKS;
    waitUntil(sample);
    if (opticalBit()) value |= (1 << i);
  }
  sample += BIT_TICKS;
  waitUntil(sample);
  bool validStop = opticalBit() == 1;
  SREG = savedSREG;
  return validStop;
}
void showMessage() {
  if (oledReady) {
    display.clear();
    display.setCursor(0, 0);
    display.print("RECEIVED");
    display.setBigCursor(0, 24);
    display.printBig(message);
  }
  Serial.print(F("RECEIVED (no CRC): "));
  Serial.println(message);
}
void processByte(byte value) {
  if (value == STX) {
    receiving = true;
    messageLength = 0;
    message[0] = '\0';
  } else if (value == ETX) {
    if (receiving && messageLength) showMessage();
    receiving = false;
  } else if (receiving) {
    if (messageLength >= MAX_MESSAGE || value < 32 || value > 126) {
      receiving = false; // Reject overflow/nonprintable payload, not a CRC check.
      return;
    }
    message[messageLength++] = (char)value;
    message[messageLength] = '\0';
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT);
  TCCR1A = 0;
  TCCR1B = _BV(CS11);
  TIMSK1 = 0;
  TCNT1 = 0;
  oledReady = display.begin();
  if (oledReady) {
    display.setCursor(0, 0);
    display.print("OPTICAL RX");
    display.setBigCursor(15, 24);
    display.printBig("WAITING");
  } else Serial.println(F("OLED not found at 0x3C. Check wiring/address."));
  Serial.println(F("Optical RX: nominal 20 kbit/s."));
  Serial.flush();
}
void loop() {
  static byte previousBit = 0; // Require observed optical idle before first start.
  byte currentBit = opticalBit();
  if (previousBit == 1 && currentBit == 0) {
    uint16_t edgeTime = TCNT1;
    byte value;
    if (receiveByte(value, edgeTime)) {
      lastByteTime = millis();
      processByte(value);
    } else receiving = false;
    previousBit = opticalBit();
  } else previousBit = currentBit;
  if (receiving && (uint32_t)(millis() - lastByteTime) > 30) receiving = false;
}
