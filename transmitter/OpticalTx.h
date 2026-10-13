#pragma once

#include <Arduino.h>
#include <avr/interrupt.h>

#if !defined(__AVR_ATmega328P__) || F_CPU != 16000000UL
#error "Use a classic 16 MHz ATmega328P UNO/Nano"
#endif

namespace OpticalTx {

static const uint16_t BIT_TICKS = 100;  // 50 us with Timer1 at 2 MHz

inline void waitUntil(uint16_t target) {
  while ((int16_t)(TCNT1 - target) < 0) {}
}

inline void begin() {
  digitalWrite(8, HIGH);  // idle optical state
  pinMode(8, OUTPUT);

  TCCR1A = 0;
  TCCR1B = _BV(CS11);    // 16 MHz / 8 = 2 MHz
  TIMSK1 = 0;
  TCNT1 = 0;
}

inline void sendByte(uint8_t value) {
  const uint8_t savedSREG = SREG;
  cli();

  // D8 = PB0. Start bit is optical 0.
  PORTB &= ~_BV(PB0);
  uint16_t next = TCNT1 + BIT_TICKS;

  for (uint8_t i = 0; i < 8; i++) {
    waitUntil(next);
    if (value & (1 << i)) PORTB |= _BV(PB0);
    else PORTB &= ~_BV(PB0);
    next += BIT_TICKS;
  }

  waitUntil(next);
  PORTB |= _BV(PB0);     // stop bit = optical 1
  next += BIT_TICKS;
  waitUntil(next);

  SREG = savedSREG;
  delayMicroseconds(150);
}

}  // namespace OpticalTx
