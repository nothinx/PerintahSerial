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
