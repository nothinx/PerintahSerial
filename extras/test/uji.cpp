// Uji logika PerintahSerial di PC:
//   g++ -std=c++11 -Wall -Wextra -I. -I../../src uji.cpp ../../src/PerintahSerial.cpp -o uji && ./uji
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "PerintahSerial.h"

static int jumlahUji = 0;
#define CEK(x) do { jumlahUji++; assert(x); } while (0)

int main() {
  { // akhiran LF, CR, dan CRLF; LF dari CRLF tidak menjadi baris kosong
    Stream s;
    PerintahSerial p(s);
    s.kirim("A\nB\rC\r\nD\r\n\r\n\n");
    CEK(p.ada() && p.adalah("A"));
    CEK(p.ada() && p.adalah("B"));
    CEK(p.ada() && p.adalah("C"));
    CEK(p.ada() && p.adalah("D"));
    CEK(!p.ada() && !p.ada());
    CEK(!p.terlaluPanjang());
  }
  { // karakter datang sedikit-sedikit di antara panggilan ada()
    Stream s;
    PerintahSerial p(s);
    s.kirim("LE");
    CEK(!p.ada());
    CEK(!p.adalah("LE"));      // baris belum selesai
    CEK(*p.teks(0) == 0 && *p.baris() == 0);
    s.kirim("D O");
    CEK(!p.ada());
    s.kirim("N\r");
    CEK(p.ada());
    CEK(p.adalah("LED") && p.adalah("ON", 1));
    s.kirim("\n");
    CEK(!p.ada());              // LF sisa CRLF diabaikan
    CEK(!p.adalah("LED"));      // baris lama sudah tidak berlaku
  }
  { // huruf besar/kecil, kata harus utuh, F()
    Stream s;
    PerintahSerial p(s);
    s.kirim("led On\n");
    CEK(p.ada());
    CEK(p.adalah("LED") && p.adalah("led") && p.adalah("Led"));
    CEK(p.adalah(F("LED")) && p.adalah(F("on"), 1));
    CEK(!p.adalah("LE") && !p.adalah("LEDS") && !p.adalah("ON"));
    CEK(!p.adalah("ON", 2) && !p.adalah("", 1));
  }
  { // argumen: angka, negatif, desimal, bukan angka, tidak ada
    Stream s;
    PerintahSerial p(s);
    s.kirim("SET 42 -17 2.5 -0.125 abc 12abc +8 99999\n");
    CEK(p.ada());
    CEK(p.jumlahArgumen() == 8);
    CEK(p.angka(1) == 42 && p.adaAngka(1));
    CEK(p.angka(2) == -17);
    CEK(!p.adaAngka(3) && p.angka(3, -1) == -1); // 2.5 bukan bilangan bulat
    CEK(p.desimal(3) == 2.5f);
    CEK(p.desimal(4) == -0.125f);
    CEK(p.desimal(2) == -17.0f);
    CEK(!p.adaAngka(5) && p.angka(5, 7) == 7 && p.desimal(5, 1.5f) == 1.5f);
    CEK(!p.adaAngka(6) && p.desimal(6, -1) == -1);
    CEK(p.angka(7) == 8);
    CEK(p.angka(8) == 99999);
    CEK(!p.adaAngka(9) && p.angka(9, 3) == 3 && p.desimal(9, 4) == 4);
    CEK(!p.adaAngka(0));
  }
  { // spasi ganda, tab, koma, dan '=' sebagai pemisah
    Stream s;
    PerintahSerial p(s);
    s.kirim("   SET\t kp=2.5 ,, 100,200   \n");
    CEK(p.ada());
    CEK(p.adalah("SET") && p.jumlahArgumen() == 4);
    CEK(p.adalah("kp", 1) && p.desimal(2) == 2.5f && p.angka(3) == 100 && p.angka(4) == 200);
    CEK(strcmp(p.teks(1), "kp") == 0 && strcmp(p.teks(2), "2.5") == 0);
    CEK(strcmp(p.teks(0), "SET") == 0 && strcmp(p.teks(5), "") == 0);
    CEK(p.angka(4) == 200 && p.adalah("SET")); // tetap benar setelah teks()
    CEK(strcmp(p.baris(), "   SET  kp 2.5    100 200   ") == 0);
    CEK(strcmp(p.teks(1), "kp") == 0);          // teks() lagi setelah baris()
  }
  { // baris() sebelum teks() utuh apa adanya
    Stream s;
    PerintahSerial p(s);
    s.kirim("PESAN Halo, dunia!\n");
    CEK(p.ada());
    CEK(strcmp(p.baris(), "PESAN Halo, dunia!") == 0);
    CEK(p.jumlahArgumen() == 2);
  }
  { // baris kosong dan baris berisi spasi saja diabaikan
    Stream s;
    PerintahSerial p(s);
    s.kirim("\n\r\n   \n\t\nOK\n");
    CEK(p.ada() && p.adalah("OK") && p.jumlahArgumen() == 0);
    CEK(!p.ada());
  }
  { // karakter kontrol dibuang
    Stream s;
    PerintahSerial p(s);
    s.kirim("L\x01" "ED\x7f\n");
    CEK(p.ada());
    CEK(!p.adalah("LED")); // DEL (0x7f) tetap disimpan, hanya < 32 yang dibuang
    s.kirim("L\x02" "ED\n");
    CEK(p.ada() && p.adalah("LED"));
  }
  { // baris kepanjangan dibuang utuh, dilaporkan sekali, baris berikutnya normal
    Stream s;
    PerintahSerialUkuran<8> p(s);
    s.kirim("1234567\n");          // pas 7 karakter
    CEK(p.ada() && p.angka(0) == 1234567);
    s.kirim("12345678 LED\nLED ON\n");
    CEK(!p.ada());
    CEK(p.terlaluPanjang());
    CEK(!p.adalah("LED"));         // sisa baris panjang tidak dianggap perintah
    CEK(p.ada() && !p.terlaluPanjang());
    CEK(p.adalah("LED") && p.adalah("ON", 1));
    s.kirim("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    CEK(!p.ada() && !p.terlaluPanjang()); // belum dilaporkan sebelum akhiran tiba
    s.kirim("\r\n");
    CEK(!p.ada() && p.terlaluPanjang());
    CEK(!p.ada() && !p.terlaluPanjang()); // LF dari CRLF tidak dilaporkan lagi
  }
  { // ukuran minimum
    Stream s;
    PerintahSerialUkuran<2> p(s);
    s.kirim("A\nAB\nB\n");
    CEK(p.ada() && p.adalah("a"));
    CEK(!p.ada() && p.terlaluPanjang());
    CEK(p.ada() && p.adalah("B"));
  }
  CEK(sizeof(PerintahSerial) <= 64 + 3 * sizeof(void *) + 3);

  printf("Semua uji lolos (%d kasus, sizeof(PerintahSerial) = %u byte)\n", jumlahUji,
         (unsigned)sizeof(PerintahSerial));
  return 0;
}
