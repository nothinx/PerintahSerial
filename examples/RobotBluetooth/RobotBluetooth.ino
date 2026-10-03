// Kendalikan robot dari HP lewat modul Bluetooth HC-05 / HC-06 (9600 baud).
// Aplikasi terminal Bluetooth di HP mengirim baris perintah:
//   MAJU 200    MUNDUR 150    KIRI    KANAN    STOP
// Angka adalah kecepatan 0-255 (bawaan 200 jika tidak diisi).
//
// Sambungan HC-05 (TX modul ke RX board, RX modul ke TX board):
//   Uno / Nano      : SoftwareSerial, TX modul ke pin 10, RX modul ke pin 11
//                     (pakai pembagi tegangan 5V ke 3,3V di RX modul)
//   Mega            : Serial1, pin 19 (RX1) dan 18 (TX1)
//   ESP32           : Serial1 di GPIO 16 (RX) dan 17 (TX)
//   STM32 / lainnya : Serial1 (Bluepill: PA10 RX, PA9 TX)
//
// Di contoh ini aksi robot hanya dicetak ke Serial Monitor (115200).
// Ganti isi gerak() dengan kode driver motor (L298N, TB6612, dll).
#include <PerintahSerial.h>

#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(10, 11); // RX, TX
#else
#define bluetooth Serial1
#endif

PerintahSerial perintah(bluetooth);

void gerak(const char *arah, int kecepatan) {
  Serial.print(arah);
  Serial.print(' ');
  Serial.println(kecepatan);
  bluetooth.println("OK"); // balasan ke HP
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  bluetooth.begin(9600, SERIAL_8N1, 16, 17);
#else
  bluetooth.begin(9600);
#endif
}

void loop() {
  if (perintah.ada()) {
    int kecepatan = constrain(perintah.angka(1, 200), 0, 255);
    if (perintah.adalah("MAJU")) gerak("maju", kecepatan);
    else if (perintah.adalah("MUNDUR")) gerak("mundur", kecepatan);
    else if (perintah.adalah("KIRI")) gerak("kiri", kecepatan);
    else if (perintah.adalah("KANAN")) gerak("kanan", kecepatan);
    else if (perintah.adalah("STOP")) gerak("berhenti", 0);
    else bluetooth.println("Perintah: MAJU, MUNDUR, KIRI, KANAN, STOP");
  }
  if (perintah.terlaluPanjang()) bluetooth.println("Perintah terlalu panjang");
}
