# PerintahSerial (English)

[Bahasa Indonesia](README.md)

An Arduino library that **reads text commands** from the Serial Monitor, a Bluetooth HC-05, SoftwareSerial, or any `Stream`. Non-blocking, no callbacks, no `String`. The API and examples are in Indonesian. This page maps every function to English.

```cpp
#include <PerintahSerial.h>

PerintahSerial command(Serial);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (command.ada()) {                                       // available()
    if (command.adalah("LED") && command.adalah("ON", 1))    // is("LED"), is("ON", 1)
      digitalWrite(LED_BUILTIN, HIGH);
    if (command.adalah("SET"))
      Serial.println(command.desimal(2));                    // "SET kp 2.5" -> 2.5
  }
}
```

## Why

- Non-blocking: `ada()` only reads bytes that already arrived. No `readString()` timeout.
- No callbacks: plain `if` statements in `loop()`.
- Accepts CR, LF, and CRLF line endings, so every Serial Monitor setting except "No line ending" works. Empty lines are ignored.
- Case-insensitive: `led on` = `LED ON`.
- Arguments split on space, tab, comma, and `=`: `SET kp 2.5`, `SET kp=2.5`, `M 100,200`.
- Number parsing with defaults: `12abc` is rejected instead of read as 12.
- Overlong lines are dropped whole and reported by `terlaluPanjang()`. Their tail is never run as a command (SerialCommands 2.2.0 and SerialCmd 1.1.6 do run it, read from their source).
- Fixed buffer, 71 bytes of RAM per object on an Uno. Command names can live in flash with `F()`.

## Simulation results

<!-- simulasi:awal -->

**A Serial Monitor session**, run on a PC with a fake `Stream` and this library's code (`extras/simulasi`). Each row is one line sent; the columns are what the library read. `angka(i)` shows `default` when the argument is not a whole number.

| Typed | Note | `ada()` | `adalah("LED")` | Command `teks(0)` | Arguments `teks(i)` | `angka(i)` | `desimal(i, NAN)` | `terlaluPanjang()` |
|---|---|---|---|---|---|---|---|---|
| `LED ON\n` | LF ending | true | true | `LED` | `ON` | default | NAN | false |
| `led On\r` | CR ending, mixed case | true | true | `led` | `On` | default | NAN | false |
| `Led oN\r\n` | CRLF ending | true | true | `Led` | `oN` | default | NAN | false |
| `SET  kp   2.5\n` | double spaces | true | false | `SET` | `kp`, `2.5` | default, default | NAN, 2.5 | false |
| `SET kp=2.5\r\n` | `=` separator | true | false | `SET` | `kp`, `2.5` | default, default | NAN, 2.5 | false |
| `M 100,-200\n` | comma separator, negative | true | false | `M` | `100`, `-200` | 100, -200 | 100, -200 | false |
| `SET kp 2,5\n` | decimal with a comma | true | false | `SET` | `kp`, `2`, `5` | default, 2, 5 | NAN, 2, 5 | false |
| `KEDIP 12abc\n` | not a number | true | false | `KEDIP` | `12abc` | default | NAN | false |
| `\r\n` | empty line | false |  |  |  |  |  | false |
| `AAAA...A (70) \n` | 70 chars, 64-byte buffer | false |  |  |  |  |  | true |
| `LED OFF\n` | next line | true | true | `LED` | `OFF` | default | NAN | false |

![Timing: PerintahSerial keeps loop() running, readString() blocks for one second](extras/gambar/waktu-baca.svg)

Simulation: the line `SET kp 2.5\r\n` arrives one character at a time at 9600 baud (1.04 ms per character), with an assumed 0.1 ms of other work per `loop()`. With `ada()`, the command is ready at 11.5 ms and `loop()` ran 116 times meanwhile. `Serial.readString()` returns only after the default 1000 ms Stream timeout, at 1012.5 ms, and `loop()` is stuck until then (computed from Stream.cpp in ArduinoCore-avr 1.8.8, not measured).

Regenerate:

```sh
cd extras/simulasi
python gambar.py   # needs g++ and matplotlib
```

<!-- simulasi:akhir -->

## Function reference

| Indonesian | English | Notes |
|---|---|---|
| `PerintahSerial(stream)` | constructor | 64-byte buffer, lines up to 63 characters |
| `PerintahSerialUkuran<N>(stream)` | constructor with size | N bytes (2-255) |
| `ada()` | available | `true` when a new command line is ready; valid until the next call |
| `terlaluPanjang()` | too long | `true` once after an overlong line was dropped |
| `adalah(word, i = 0)` | is | word `i` equals `word`, case-insensitive; also takes `F("...")` |
| `jumlahArgumen()` | argument count | words after the command |
| `teks(i)` | text | word `i`, `""` if missing |
| `angka(i, bawaan = 0)` | number (long) | `bawaan` (default) if missing or not an integer |
| `adaAngka(i)` | has number | argument `i` is a valid integer |
| `desimal(i, bawaan = 0)` | decimal (float) | use a dot: `2.5` |
| `baris()` | line | the whole line |

Index 0 is the command, 1 and up are arguments.

## Examples

`KontrolLED` (LED on/off/blink), `AturNilaiLewatSerial` (tune PID values live), `RobotBluetooth` (HC-05 robot control on Uno, Mega, ESP32, STM32), `DaftarPerintah` (help menu with `F()`).

## Status

Version 1.0.0 passes automated logic tests and compiles on Uno, Mega, ESP32, ESP32-C3, ESP32-S3, STM32 Blackpill F411, and Bluepill F103. It has **not yet been tested on real hardware**.

## License

MIT © 2026 Amadeo Wisesa.
