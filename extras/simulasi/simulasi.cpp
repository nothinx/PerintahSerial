// Simulasi PerintahSerial di PC: menjalankan kode library asli (../../src)
// dengan Stream tiruan (../test/Arduino.h) dan mencetak hasilnya sebagai
// baris "bagian<TAB>kolom<TAB>...". Dipanggil oleh gambar.py.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "PerintahSerial.h"

// Tampilkan \r, \n, \t sebagai teks agar akhiran baris terlihat.
static void cetakMentah(const char *s) {
  for (; *s; s++) {
    if (*s == '\r') printf("\\r");
    else if (*s == '\n') printf("\\n");
    else if (*s == '\t') printf("\\t");
    else putchar(*s);
  }
}

// 1. Sesi Serial Monitor: kirim satu potong data, lalu panggil ada() seperti
//    di loop(). Satu baris keluaran per ada() == true; jika tidak ada baris
//    perintah sama sekali, satu baris dengan ada() == false.
//    Kolom: dikirim, catatan, ada(), terlaluPanjang(), adalah("LED"), lalu per kata
//    "teks(i)|adaAngka(i)|angka(i)|desimal(i, NAN)".
static void sesi(Stream &s, PerintahSerial &p, const char *dikirim, const char *catatan,
                 const char *tampil = nullptr) {
  s.kirim(dikirim);
  bool adaBaris = false;
  for (;;) {
    bool hasil = p.ada();
    if (!hasil && adaBaris) return; // mis. LF sisa CRLF: dibuang tanpa baris baru
    printf("sesi\t");
    if (tampil) printf("%s", tampil);
    else cetakMentah(dikirim);
    printf("\t%s\t%s\t%s", catatan, hasil ? "true" : "false", p.terlaluPanjang() ? "true" : "false");
    if (hasil) {
      printf("\t%s", p.adalah("LED") ? "true" : "false");
      for (uint8_t i = 0; i <= p.jumlahArgumen(); i++) {
        long n = p.angka(i);
        bool bulat = p.adaAngka(i);
        float x = p.desimal(i, NAN);
        printf("\t%s|%d|%ld|", p.teks(i), bulat, n);
        if (!isnan(x)) printf("%g", x);
      }
    }
    printf("\n");
    if (!hasil) return;
    adaBaris = true;
  }
}

// 2. Waktu: satu baris datang per karakter pada 9600 baud (8N1 = 10 bit per
//    karakter = 1,0417 ms), sementara loop() berputar. Asumsi: isi loop() selain
//    ada() butuh 100 us per putaran; waktu ada() sendiri diabaikan.
static void waktu() {
  const char *baris = "SET kp 2.5\r\n";
  const double MS_PER_KARAKTER = 10.0 / 9600 * 1000, MS_PER_PUTARAN = 0.1;
  Stream s;
  PerintahSerial p(s);
  size_t terkirim = 0, n = strlen(baris);
  long putaran = 0;
  double siap = -1;
  while (siap < 0) {
    double t = putaran * MS_PER_PUTARAN;
    // karakter ke-k (dari 0) selesai diterima pada (k + 1) * MS_PER_KARAKTER
    while (terkirim < n && (terkirim + 1) * MS_PER_KARAKTER <= t) {
      char c[2] = {baris[terkirim++], 0};
      s.kirim(c);
    }
    putaran++;
    if (p.ada()) siap = t;
  }
  double tibaTerakhir = n * MS_PER_KARAKTER;
  printf("waktu\tkarakter\t%u\n", (unsigned)n);
  printf("waktu\tms_per_karakter\t%.4f\n", MS_PER_KARAKTER);
  printf("waktu\tms_per_putaran\t%.1f\n", MS_PER_PUTARAN);
  printf("waktu\tsiap_ms\t%.2f\n", siap);
  printf("waktu\tputaran\t%ld\n", putaran);
  printf("waktu\tterakhir_ms\t%.2f\n", tibaTerakhir);
  // Pembanding: if (Serial.available()) s = Serial.readString();
  // Stream::readString() (ArduinoCore-avr 1.8.8, Stream.cpp) membaca per
  // karakter dengan timedRead() dan baru kembali setelah 1000 ms (timeout
  // bawaan Stream) tanpa karakter baru. loop() tertahan sejak karakter pertama
  // tiba sampai saat itu. Ini hitungan, bukan kode library.
  printf("waktu\treadstring_mulai_ms\t%.2f\n", MS_PER_KARAKTER);
  printf("waktu\treadstring_selesai_ms\t%.2f\n", tibaTerakhir + 1000);
}

int main() {
  Stream s;
  PerintahSerial p(s);
  sesi(s, p, "LED ON\n", "akhiran LF");
  sesi(s, p, "led On\r", "akhiran CR, huruf campur");
  sesi(s, p, "Led oN\r\n", "akhiran CRLF");
  sesi(s, p, "SET  kp   2.5\n", "spasi ganda");
  sesi(s, p, "SET kp=2.5\r\n", "pemisah =");
  sesi(s, p, "M 100,-200\n", "pemisah koma, negatif");
  sesi(s, p, "SET kp 2,5\n", "desimal ditulis dengan koma");
  sesi(s, p, "KEDIP 12abc\n", "bukan angka");
  sesi(s, p, "\r\n", "baris kosong");
  char panjang[80];
  memset(panjang, 'A', 70);
  strcpy(panjang + 70, "\n");
  sesi(s, p, panjang, "70 karakter, buffer 64", "AAAA...A (70) \\n");
  sesi(s, p, "LED OFF\n", "baris berikutnya");
  waktu();
  return 0;
}
