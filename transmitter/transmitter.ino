// Optical keypad transmitter: classic 16 MHz ATmega328P UNO.
#include <Arduino.h>
#include <avr/interrupt.h>
#if !defined(__AVR_ATmega328P__) || F_CPU != 16000000UL
#error "Use a classic 16 MHz ATmega328P UNO/Nano"
#endif
const uint16_t BIT_TICKS = 100; // Timer1 / 8 = 2 MHz; 100 ticks = 50 us.
const byte STX = 0x02, ETX = 0x03, MAX_MESSAGE = 16;
const byte rowPins[4] = {11, 10, 9, 7};
const byte colPins[4] = {6, 5, 4, 3};
const char keyMap[4][4] = {
  {'1','2','3','A'}, {'4','5','6','B'},
  {'7','8','9','C'}, {'*','0','I','H'}
};
char message[MAX_MESSAGE + 1] = {};
char previousMessage[MAX_MESSAGE + 1] = {};
byte messageLength = 0, previousLength = 0;

void waitUntil(uint16_t tick) {
  while ((int16_t)(TCNT1 - tick) < 0) {}
}
void sendByteOptically(byte value) {
  const byte savedSREG = SREG;
  cli();
  // Direct D8 (PB0) access and absolute deadlines avoid cumulative delays.
  PORTB &= ~_BV(PB0);
  uint16_t next = TCNT1 + BIT_TICKS;
  for (byte i = 0; i < 8; i++) {
    waitUntil(next);
    if (value & (1 << i)) PORTB |= _BV(PB0);
    else PORTB &= ~_BV(PB0);
    next += BIT_TICKS;
  }
  waitUntil(next);
  PORTB |= _BV(PB0); // Stop bit, optical 1.
  next += BIT_TICKS;
  waitUntil(next);
  SREG = savedSREG;
  delayMicroseconds(150); // Three idle bit periods, plus software overhead.
}
char scanKeypad() {
  // Inactive rows float, avoiding output contention on multiple key presses.
  for (byte r = 0; r < 4; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(5);
    for (byte c = 0; c < 4; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        pinMode(rowPins[r], INPUT);
        return keyMap[r][c];
      }
    }
    pinMode(rowPins[r], INPUT);
  }
  return 0;
}
void sendWholeMessage() {
  if (messageLength) {
    previousLength = messageLength;
    memcpy(previousMessage, message, messageLength + 1);
  }
  if (!previousLength) {
    Serial.println(F("No previous message to send."));
    return;
  }
  Serial.print(F("TRANSMITTING: "));
  Serial.println(previousMessage);
  Serial.flush(); // Finish UART output before the timing-critical section.
  sendByteOptically(STX);
  for (byte i = 0; i < previousLength; i++) sendByteOptically(previousMessage[i]);
  sendByteOptically(ETX);
  messageLength = 0;
  message[0] = '\0';
  delay(150); // Give receiver time to update its OLED before another packet.
  Serial.println(F("Transmitted. Press * to repeat."));
}
void setup() {
  Serial.begin(115200);
  digitalWrite(8, HIGH);
  pinMode(8, OUTPUT);
  TCCR1A = 0;
  TCCR1B = _BV(CS11);
  TIMSK1 = 0;
  TCNT1 = 0;
  for (byte r = 0; r < 4; r++) {
    digitalWrite(rowPins[r], LOW);
    pinMode(rowPins[r], INPUT);
  }
  for (byte c = 0; c < 4; c++) pinMode(colPins[c], INPUT_PULLUP);
  Serial.println(F("Optical TX: nominal 20 kbit/s. * sends; # types I; D types H."));
}
void loop() {
  char key = scanKeypad();
  if (!key) return;
  delay(25);
  if (scanKeypad() != key) return;
  if (key == '*') sendWholeMessage();
  else if (messageLength < MAX_MESSAGE) {
    message[messageLength++] = key;
    message[messageLength] = '\0';
    Serial.print(F("TYPING: "));
    Serial.println(message);
  } else Serial.println(F("Message full (16 characters)."));
  while (scanKeypad()) delay(5);
  delay(20);
}
