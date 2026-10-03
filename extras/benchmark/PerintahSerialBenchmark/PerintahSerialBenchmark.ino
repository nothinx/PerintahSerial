// Benchmark PerintahSerial di ATmega328P 16 MHz (simavr). Cara menjalankan dan
// angka hasilnya: README bagian "Kecepatan & memori".
// Siklus.h: Timer1 tanpa prescaler, UKUR(nama, ulang, kode) mencetak
// "BENCH nama siklus_per_panggilan". Baris perintah dibaca dari Stream di
// memori, jadi yang terukur adalah library, bukan kecepatan UART.
#include <PerintahSerial.h>
#include "Siklus.h"

struct Pita : Stream {
  const char *p = "";
  int available() override { return *p ? 1 : 0; }
  int read() override { return *p ? *p++ : -1; }
  int peek() override { return *p ? *p : -1; }
  size_t write(uint8_t) override { return 1; }
} pita;

PerintahSerial perintah(pita);
volatile float kp;
volatile uint8_t n;

// Lima perintah; "SET" di urutan ketiga.
void layani() {
  if (!perintah.ada()) return;
  if (perintah.adalah(F("LED"))) n++;
  else if (perintah.adalah(F("MOTOR"))) n++;
  else if (perintah.adalah(F("SET"))) { if (perintah.adalah(F("KP"), 1)) kp = perintah.desimal(2); }
  else if (perintah.adalah(F("GET"))) n++;
  else if (perintah.adalah(F("HELP"))) n++;
}

void setup() {
  Serial.begin(115200);
  Serial.print(F("BENCH sizeof "));
  Serial.println(sizeof(PerintahSerial));
  UKUR("tanpa_data", 1000, layani());
  UKUR("baris_SET_KP_2.5", 100, { pita.p = "SET KP 2.5\n"; layani(); });
  selesai();
}

void loop() {}
