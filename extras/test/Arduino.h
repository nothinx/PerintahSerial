// Arduino.h tiruan untuk menguji logika PerintahSerial di PC.
#pragma once
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define pgm_read_byte(p) (*(const uint8_t *)(p))
class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

// Stream tiruan: data dimasukkan lewat kirim(), dibaca seperti Serial.
class Stream {
public:
  void kirim(const char *s) { strcat(_isi, s); }
  int available() { return (int)(strlen(_isi) - _baca); }
  int read() { return available() > 0 ? (uint8_t)_isi[_baca++] : -1; }

private:
  char _isi[2000] = "";
  size_t _baca = 0;
};
