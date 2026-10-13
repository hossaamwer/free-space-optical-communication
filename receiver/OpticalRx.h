#pragma once

#include <Arduino.h>
#include <avr/interrupt.h>

#if !defined(__AVR_ATmega328P__) || F_CPU != 16000000UL
#error "Use a classic 16 MHz ATmega328P UNO/Nano"
#endif

namespace OpticalRx {

static const uint16_t BIT_TICKS = 100;  // 50 us with Timer1 at 2 MHz

inline uint8_t opticalBit() {
  return (PIND & _BV(PD2)) ? 0 : 1;  // receiver output is active-low
}

inline void waitUntil(uint16_t target) {
  while ((int16_t)(TCNT1 - target) < 0) {}
}

inline bool readByte(uint8_t &value, uint16_t edgeTime) {
  const uint8_t savedSREG = SREG;
  cli();

  uint16_t sample = edgeTime + BIT_TICKS / 2;
  waitUntil(sample);

  if (opticalBit() != 0) {
    SREG = savedSREG;
    return false;
  }

  value = 0;
  for (uint8_t i = 0; i < 8; i++) {
    sample += BIT_TICKS;
    waitUntil(sample);
    if (opticalBit()) value |= (1 << i);
  }

  sample += BIT_TICKS;
  waitUntil(sample);
  const bool validStop = opticalBit() == 1;

  SREG = savedSREG;
  return validStop;
}

inline void begin() {
  pinMode(2, INPUT);
  TCCR1A = 0;
  TCCR1B = _BV(CS11);    // 16 MHz / 8 = 2 MHz
  TIMSK1 = 0;
  TCNT1 = 0;
}

inline bool poll(uint8_t &value) {
  static uint8_t previousBit = 0;
  const uint8_t currentBit = opticalBit();

  if (previousBit == 1 && currentBit == 0) {
    const uint16_t edgeTime = TCNT1;
    const bool ok = readByte(value, edgeTime);
    previousBit = opticalBit();
    return ok;
  }

  previousBit = currentBit;
  return false;
}

}  // namespace OpticalRx
