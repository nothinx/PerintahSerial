// PerintahSerial - membaca perintah teks dari Serial Monitor, Bluetooth HC-05,
// SoftwareSerial, atau Stream apa pun. Non-blocking, tanpa callback.
// Copyright (c) 2026 Amadeo Wisesa. Lisensi MIT.
//
// - Akhiran baris CR, LF, atau CRLF (semua pilihan Serial Monitor selain
//   "No line ending"). Baris kosong diabaikan.
// - Perintah tidak membedakan huruf besar/kecil: "led on" = "LED ON".
// - Pemisah argumen: spasi, tab, koma, dan '='. "SET kp=2.5" = "SET kp 2.5".
// - Buffer tetap, tanpa String dan tanpa alokasi dinamis. Baris yang terlalu
//   panjang dibuang utuh dan dilaporkan lewat terlaluPanjang().
#pragma once
#include <Arduino.h>

class PerintahSerialDasar {
public:
  // Panggil di setiap loop(). true jika ada satu baris perintah baru.
  // Baris itu berlaku sampai ada() dipanggil lagi.
  bool ada();
  // true (sekali) jika baris terakhir lebih panjang dari buffer dan dibuang.
  bool terlaluPanjang() const { return _status & S_PANJANG; }

  // true jika kata ke-i sama dengan kata (huruf besar/kecil diabaikan).
  // i = 0 adalah perintahnya, 1 dan seterusnya argumen.
  // Contoh: adalah("LED") atau adalah("ON", 1).
  bool adalah(const char *kata, uint8_t i = 0) const;
  bool adalah(const __FlashStringHelper *kata, uint8_t i = 0) const; // adalah(F("LED")) hemat RAM di AVR

  uint8_t jumlahArgumen() const;  // jumlah kata setelah perintah
  const char *teks(uint8_t i);    // kata ke-i, "" jika tidak ada
  // Argumen ke-i sebagai bilangan bulat. bawaan jika tidak ada atau bukan angka.
  long angka(uint8_t i, long bawaan = 0) const;
  bool adaAngka(uint8_t i) const; // true jika argumen ke-i bilangan bulat yang valid
  // Argumen ke-i sebagai desimal (pakai titik: 2.5). bawaan jika tidak valid.
  float desimal(uint8_t i, float bawaan = 0) const;
  // Seluruh baris. Jika teks() sudah dipanggil, pemisah tab, koma, dan '=' menjadi spasi.
  const char *baris();

protected:
  PerintahSerialDasar(Stream &stream, char *buffer, uint8_t ukuran)
    : _stream(stream), _buf(buffer), _ukuran(ukuran) {}

private:
  enum : uint8_t { S_SIAP = 1, S_BUANG = 2, S_PANJANG = 4 };
  const char *kata(uint8_t i) const;
  bool bacaAngka(uint8_t i, long &n) const;
  bool sama(const char *kata, uint8_t i, bool flash) const;

  Stream &_stream;
  char *_buf;
  uint8_t _ukuran, _panjang = 0, _status = 0;
};

// Ukuran buffer bisa diatur: PerintahSerialUkuran<128> perintah(Serial);
// Baris terpanjang = UKURAN - 1 karakter.
template <uint8_t UKURAN>
class PerintahSerialUkuran : public PerintahSerialDasar {
public:
  PerintahSerialUkuran(Stream &stream) : PerintahSerialDasar(stream, _isi, UKURAN) {}

private:
  char _isi[UKURAN];
};

// Bawaan: baris sampai 63 karakter. PerintahSerial perintah(Serial);
typedef PerintahSerialUkuran<64> PerintahSerial;
