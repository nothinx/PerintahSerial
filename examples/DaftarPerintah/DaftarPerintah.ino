// Menu perintah dengan BANTUAN, untuk debugging lewat Serial Monitor
// (115200, akhiran "New Line"). Nama perintah ditulis dengan F() agar
// tersimpan di flash, bukan RAM. Ini penting di Uno jika perintahnya banyak.
#include <PerintahSerial.h>

PerintahSerial perintah(Serial);

void bantuan() {
  Serial.println(F("Daftar perintah:"));
  Serial.println(F("  BANTUAN             tampilkan daftar ini"));
  Serial.println(F("  BACA <pin>          baca pin analog, misalnya BACA 0"));
  Serial.println(F("  TULIS <pin> <0|1>   tulis pin digital, misalnya TULIS 13 1"));
  Serial.println(F("  WAKTU               lama menyala (ms)"));
}

void setup() {
  Serial.begin(115200);
  bantuan();
}

void loop() {
  if (perintah.ada()) {
    if (perintah.adalah(F("BANTUAN")) || perintah.adalah(F("?"))) {
      bantuan();
    } else if (perintah.adalah(F("BACA")) && perintah.adaAngka(1)) {
      Serial.println(analogRead(perintah.angka(1)));
    } else if (perintah.adalah(F("TULIS")) && perintah.jumlahArgumen() == 2) {
      uint8_t pin = perintah.angka(1);
      pinMode(pin, OUTPUT);
      digitalWrite(pin, perintah.angka(2) ? HIGH : LOW);
      Serial.println(F("OK"));
    } else if (perintah.adalah(F("WAKTU"))) {
      Serial.println(millis());
    } else {
      Serial.print(F("Tidak dikenal: "));
      Serial.print(perintah.teks(0));
      Serial.println(F(". Ketik BANTUAN."));
    }
  }
  if (perintah.terlaluPanjang()) Serial.println(F("Baris terlalu panjang, dibuang."));
}
