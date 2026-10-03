// Kendalikan LED bawaan dari Serial Monitor (115200, akhiran "New Line").
// Perintah (huruf besar/kecil sama saja):
//   LED ON       nyalakan
//   LED OFF      matikan
//   KEDIP 200    berkedip tiap 200 ms (KEDIP 0 = berhenti)
#include <PerintahSerial.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // ESP32 DevKit
#endif

PerintahSerial perintah(Serial);
uint32_t jedaKedip = 0, waktuKedip = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Ketik: LED ON, LED OFF, atau KEDIP 200");
}

void loop() {
  if (perintah.ada()) {
    if (perintah.adalah("LED") && perintah.adalah("ON", 1)) {
      jedaKedip = 0;
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED menyala");
    } else if (perintah.adalah("LED") && perintah.adalah("OFF", 1)) {
      jedaKedip = 0;
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED mati");
    } else if (perintah.adalah("KEDIP") && perintah.adaAngka(1)) {
      jedaKedip = perintah.angka(1);
      Serial.print("Berkedip tiap ");
      Serial.print(jedaKedip);
      Serial.println(" ms");
    } else {
      Serial.print("Perintah tidak dikenal: ");
      Serial.println(perintah.baris());
    }
  }

  // Berkedip tanpa delay(), jadi perintah baru tetap langsung terbaca.
  if (jedaKedip && millis() - waktuKedip >= jedaKedip) {
    waktuKedip = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
