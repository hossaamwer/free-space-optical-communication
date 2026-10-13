#pragma once
#include <Wire.h>
#include <avr/pgmspace.h>
class MiniOLED {
public:
  MiniOLED(uint8_t address = 0x3C) {
    _address = address;
  }
  bool begin() {
    Wire.begin();
    Wire.beginTransmission(_address);
    if (Wire.endTransmission() != 0) {
      return false;
    }
    command(0xAE);
    command2(0xD5, 0x80);
    command2(0xA8, 0x3F);
    command2(0xD3, 0x00);
    command(0x40);
    command2(0x8D, 0x14);
    command2(0x20, 0x02);
    command(0xA1);
    command(0xC8);
    command2(0xDA, 0x12);
    command2(0x81, 0x7F);
    command2(0xD9, 0xF1);
    command2(0xDB, 0x40);
    command(0xA4);
    command(0xA6);
    command(0xAF);
    clear();
    return true;
  }
  void clear() {
    for (byte page = 0; page < 8; page++) {
      setPageColumn(page, 0);
      for (byte block = 0; block < 8; block++) {
        Wire.beginTransmission(_address);
        Wire.write(0x40);
        for (byte i = 0; i < 16; i++) Wire.write((uint8_t)0);
        Wire.endTransmission();
      }
    }
  }
  void setCursor(byte x, byte y) {
    smallX = x;
    smallPage = y / 8;
    setPageColumn(smallPage, smallX);
  }
  void print(const char *text) {
    while (*text) print(*text++);
  }
  void print(char c) {
    if (smallX > 122) {
      smallX = 0;
      smallPage++;
    }
    setPageColumn(smallPage, smallX);
    Wire.beginTransmission(_address);
    Wire.write(0x40);
    for (byte i = 0; i < 5; i++) Wire.write(glyph(c, i));
    Wire.write((uint8_t)0);
    Wire.endTransmission();
    smallX += 6;
  }
  void setBigCursor(byte x, byte y) {
    bigX = x;
    bigPage = y / 8;
  }
  void printBig(const char *text) {
    while (*text) printBig(*text++);
  }
  void printBig(char c) {
    if (bigX > 116) {
      bigX = 0;
      bigPage += 2;
    }
    if (bigPage > 6) return;
    uint16_t scaledColumns[6];
    for (byte column = 0; column < 5; column++) {
      byte original = glyph(c, column);
      uint16_t scaled = 0;
      for (byte bit = 0; bit < 8; bit++) {
        if (original & (1 << bit)) {
          scaled |= (1UL << (bit * 2));
          scaled |= (1UL << (bit * 2 + 1));
        }
      }
      scaledColumns[column] = scaled;
    }
    scaledColumns[5] = 0;
    setPageColumn(bigPage, bigX);
    Wire.beginTransmission(_address);
    Wire.write(0x40);
    for (byte column = 0; column < 6; column++) {
      byte top = scaledColumns[column] & 0xFF;
      Wire.write(top);
      Wire.write(top);
    }
    Wire.endTransmission();
    setPageColumn(bigPage + 1, bigX);
    Wire.beginTransmission(_address);
    Wire.write(0x40);
    for (byte column = 0; column < 6; column++) {
      byte bottom = (scaledColumns[column] >> 8) & 0xFF;
      Wire.write(bottom);
      Wire.write(bottom);
    }
    Wire.endTransmission();
    bigX += 12;
  }
private:
  uint8_t _address;
  byte smallX = 0;
  byte smallPage = 0;
  byte bigX = 0;
  byte bigPage = 0;
  void command(uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    Wire.write(value);
    Wire.endTransmission();
  }
  void command2(uint8_t a, uint8_t b) {
    command(a);
    command(b);
  }
  void setPageColumn(byte page, byte column) {
    command(0xB0 | page);
    command(column & 0x0F);
    command(0x10 | ((column >> 4) & 0x0F));
  }
  uint8_t glyph(char c, byte i) {
    if (c >= 'a' && c <= 'z') c -= 32;
    static const uint8_t SPACE[] PROGMEM = {0,0,0,0,0};
    static const uint8_t QUESTION[] PROGMEM = {0x02,0x01,0x51,0x09,0x06};
    static const uint8_t N0[] PROGMEM = {0x3E,0x51,0x49,0x45,0x3E};
    static const uint8_t N1[] PROGMEM = {0x00,0x42,0x7F,0x40,0x00};
    static const uint8_t N2[] PROGMEM = {0x42,0x61,0x51,0x49,0x46};
    static const uint8_t N3[] PROGMEM = {0x21,0x41,0x45,0x4B,0x31};
    static const uint8_t N4[] PROGMEM = {0x18,0x14,0x12,0x7F,0x10};
    static const uint8_t N5[] PROGMEM = {0x27,0x45,0x45,0x45,0x39};
    static const uint8_t N6[] PROGMEM = {0x3C,0x4A,0x49,0x49,0x30};
    static const uint8_t N7[] PROGMEM = {0x01,0x71,0x09,0x05,0x03};
    static const uint8_t N8[] PROGMEM = {0x36,0x49,0x49,0x49,0x36};
    static const uint8_t N9[] PROGMEM = {0x06,0x49,0x49,0x29,0x1E};
    static const uint8_t A[] PROGMEM = {0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t B[] PROGMEM = {0x7F,0x49,0x49,0x49,0x36};
    static const uint8_t C[] PROGMEM = {0x3E,0x41,0x41,0x41,0x22};
    static const uint8_t D[] PROGMEM = {0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t E[] PROGMEM = {0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t F[] PROGMEM = {0x7F,0x09,0x09,0x09,0x01};
    static const uint8_t G[] PROGMEM = {0x3E,0x41,0x49,0x49,0x7A};
    static const uint8_t H[] PROGMEM = {0x7F,0x08,0x08,0x08,0x7F};
    static const uint8_t I[] PROGMEM = {0x00,0x41,0x7F,0x41,0x00};
    static const uint8_t J[] PROGMEM = {0x20,0x40,0x41,0x3F,0x01};
    static const uint8_t K[] PROGMEM = {0x7F,0x08,0x14,0x22,0x41};
    static const uint8_t L[] PROGMEM = {0x7F,0x40,0x40,0x40,0x40};
    static const uint8_t M[] PROGMEM = {0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t N[] PROGMEM = {0x7F,0x04,0x08,0x10,0x7F};
    static const uint8_t O[] PROGMEM = {0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t P[] PROGMEM = {0x7F,0x09,0x09,0x09,0x06};
    static const uint8_t Q[] PROGMEM = {0x3E,0x41,0x51,0x21,0x5E};
    static const uint8_t R[] PROGMEM = {0x7F,0x09,0x19,0x29,0x46};
    static const uint8_t S[] PROGMEM = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[] PROGMEM = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t U[] PROGMEM = {0x3F,0x40,0x40,0x40,0x3F};
    static const uint8_t V[] PROGMEM = {0x1F,0x20,0x40,0x20,0x1F};
    static const uint8_t W[] PROGMEM = {0x3F,0x40,0x38,0x40,0x3F};
    static const uint8_t X[] PROGMEM = {0x63,0x14,0x08,0x14,0x63};
    static const uint8_t Y[] PROGMEM = {0x07,0x08,0x70,0x08,0x07};
    static const uint8_t Z[] PROGMEM = {0x61,0x51,0x49,0x45,0x43};
    const uint8_t *selected = QUESTION;
    switch (c) {
      case ' ': selected = SPACE; break;
      case '0': selected = N0; break;
      case '1': selected = N1; break;
      case '2': selected = N2; break;
      case '3': selected = N3; break;
      case '4': selected = N4; break;
      case '5': selected = N5; break;
      case '6': selected = N6; break;
      case '7': selected = N7; break;
      case '8': selected = N8; break;
      case '9': selected = N9; break;
      case 'A': selected = A; break;
      case 'B': selected = B; break;
      case 'C': selected = C; break;
      case 'D': selected = D; break;
      case 'E': selected = E; break;
      case 'F': selected = F; break;
      case 'G': selected = G; break;
      case 'H': selected = H; break;
      case 'I': selected = I; break;
      case 'J': selected = J; break;
      case 'K': selected = K; break;
      case 'L': selected = L; break;
      case 'M': selected = M; break;
      case 'N': selected = N; break;
      case 'O': selected = O; break;
      case 'P': selected = P; break;
      case 'Q': selected = Q; break;
      case 'R': selected = R; break;
      case 'S': selected = S; break;
      case 'T': selected = T; break;
      case 'U': selected = U; break;
      case 'V': selected = V; break;
      case 'W': selected = W; break;
      case 'X': selected = X; break;
      case 'Y': selected = Y; break;
      case 'Z': selected = Z; break;
    }
    return pgm_read_byte(selected + i);
  }
};
