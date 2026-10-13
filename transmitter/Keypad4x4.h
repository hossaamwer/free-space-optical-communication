#pragma once

#include <Arduino.h>

namespace Keypad4x4 {

static const uint8_t rowPins[4] = {11, 10, 9, 7};
static const uint8_t colPins[4] = {6, 5, 4, 3};
static const char keyMap[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

inline void begin() {
  for (uint8_t r = 0; r < 4; r++) {
    digitalWrite(rowPins[r], LOW);
    pinMode(rowPins[r], INPUT);
  }

  for (uint8_t c = 0; c < 4; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }
}

inline char read() {
  for (uint8_t r = 0; r < 4; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(5);

    for (uint8_t c = 0; c < 4; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        pinMode(rowPins[r], INPUT);
        return keyMap[r][c];
      }
    }

    pinMode(rowPins[r], INPUT);
  }

  return 0;
}

}  // namespace Keypad4x4
