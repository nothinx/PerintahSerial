// Ubah konstanta PID (atau nilai apa pun) lewat Serial Monitor tanpa
// upload ulang. Cocok untuk tuning robot line follower atau self-balancing.
//
// Perintah (115200, akhiran "New Line"):
//   SET kp 2.5     atau SET kp=2.5
//   SET ki 0.1
//   SET kd 12
//   LIHAT          tampilkan semua nilai
#include <PerintahSerial.h>

PerintahSerial perintah(Serial);
float kp = 1.0, ki = 0.0, kd = 0.0;

void tampilkan() {
  Serial.print("kp = ");
  Serial.print(kp, 3);
  Serial.print("  ki = ");
  Serial.print(ki, 3);
  Serial.print("  kd = ");
  Serial.println(kd, 3);
}

void setup() {
  Serial.begin(115200);
  tampilkan();
}

void loop() {
  if (perintah.ada()) {
    if (perintah.adalah("SET")) {
      // NAN sebagai bawaan: tanda bahwa argumen ke-2 bukan angka.
      float nilai = perintah.desimal(2, NAN);
      if (isnan(nilai)) {
        Serial.println("Format: SET kp 2.5");
      } else if (perintah.adalah("kp", 1)) {
        kp = nilai;
      } else if (perintah.adalah("ki", 1)) {
        ki = nilai;
      } else if (perintah.adalah("kd", 1)) {
        kd = nilai;
      } else {
        Serial.print("Tidak ada nilai bernama ");
        Serial.println(perintah.teks(1));
      }
      tampilkan();
    } else if (perintah.adalah("LIHAT")) {
      tampilkan();
    } else {
      Serial.println("Perintah: SET <kp|ki|kd> <nilai>, LIHAT");
    }
  }

  // ... hitung PID dengan kp, ki, kd di sini ...
}
