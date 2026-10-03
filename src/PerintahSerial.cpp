#include "PerintahSerial.h"

static bool pemisah(char c) { return c == ' ' || c == '\t' || c == ',' || c == '=' || c == 0; }

bool PerintahSerialDasar::ada() {
  if (_status & S_SIAP) _panjang = 0; // baris sebelumnya sudah selesai dipakai
  _status &= S_BUANG;
  while (_stream.available() > 0) {
    char c = _stream.read();
    if (c == '\r' || c == '\n') {
      if (_status & S_BUANG) {
        _status = S_PANJANG;
        _panjang = 0;
        return false;
      }
      _buf[_panjang] = 0;
      _status = S_SIAP;
      if (kata(0)) return true; // berhenti membaca: sisa data untuk ada() berikutnya
      _status = 0;              // baris kosong, termasuk LF dari CRLF
      _panjang = 0;
    } else if ((uint8_t)c >= ' ' || c == '\t') {
      if (_panjang < _ukuran - 1) _buf[_panjang++] = c;
      else _status = S_BUANG;   // buang sisa baris sampai akhiran
    }
  }
  return false;
}

const char *PerintahSerialDasar::kata(uint8_t i) const {
  if (!(_status & S_SIAP)) return nullptr;
  for (uint8_t j = 0; j < _panjang; j++) {
    if (!pemisah(_buf[j]) && (j == 0 || pemisah(_buf[j - 1])) && i-- == 0) return _buf + j;
  }
  return nullptr;
}

bool PerintahSerialDasar::sama(const char *cari, uint8_t i, bool flash) const {
  const char *p = kata(i);
  if (!p) return false;
  for (;; p++, cari++) {
    char c = flash ? pgm_read_byte(cari) : *cari;
    if (!c) return pemisah(*p);
    if (tolower((uint8_t)c) != tolower((uint8_t)*p)) return false;
  }
}

bool PerintahSerialDasar::adalah(const char *kata, uint8_t i) const { return sama(kata, i, false); }

bool PerintahSerialDasar::adalah(const __FlashStringHelper *kata, uint8_t i) const {
  return sama(reinterpret_cast<const char *>(kata), i, true);
}

uint8_t PerintahSerialDasar::jumlahArgumen() const {
  uint8_t n = 0;
  while (kata(n)) n++;
  return n ? n - 1 : 0;
}

const char *PerintahSerialDasar::teks(uint8_t i) {
  if (!(_status & S_SIAP)) return "";
  for (uint8_t j = 0; j < _panjang; j++) {
    if (pemisah(_buf[j])) _buf[j] = 0; // potong menjadi kata-kata
  }
  const char *p = kata(i);
  return p ? p : "";
}

const char *PerintahSerialDasar::baris() {
  if (!(_status & S_SIAP)) return "";
  for (uint8_t j = 0; j < _panjang; j++) {
    if (!_buf[j]) _buf[j] = ' '; // sambung lagi setelah teks()
  }
  return _buf;
}

bool PerintahSerialDasar::bacaAngka(uint8_t i, long &n) const {
  const char *p = kata(i);
  if (!p) return false;
  char *akhir;
  n = strtol(p, &akhir, 10);
  return akhir != p && pemisah(*akhir); // "12abc" bukan angka
}

long PerintahSerialDasar::angka(uint8_t i, long bawaan) const {
  long n;
  return bacaAngka(i, n) ? n : bawaan;
}

bool PerintahSerialDasar::adaAngka(uint8_t i) const {
  long n;
  return bacaAngka(i, n);
}

float PerintahSerialDasar::desimal(uint8_t i, float bawaan) const {
  const char *p = kata(i);
  if (!p) return bawaan;
  char *akhir;
  float x = strtod(p, &akhir);
  return (akhir != p && pemisah(*akhir)) ? x : bawaan;
}
