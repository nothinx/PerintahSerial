"""Bangkitkan hasil simulasi PerintahSerial dari kode library asli.

Jalankan dari folder ini: python gambar.py   (butuh g++ dan matplotlib)
- compile + jalankan simulasi.cpp (memakai ../test/Arduino.h dan ../../src),
- tulis tabel sesi Serial Monitor ke README.md dan README.en.md (di antara
  penanda <!-- simulasi:awal --> dan <!-- simulasi:akhir -->),
- render ../gambar/waktu-baca.svg.
"""
import os
import re
import subprocess
import sys
import tempfile
from collections import defaultdict
from glob import glob

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
plt.rcParams.update({
    "figure.figsize": (8, 3.6), "figure.dpi": 100, "savefig.bbox": "tight", "savefig.pad_inches": 0.15,
    "figure.facecolor": "white", "axes.facecolor": "white", "savefig.facecolor": "white",
    "font.size": 10, "axes.titlesize": 11, "axes.titleweight": "bold", "axes.titlelocation": "left",
    "axes.spines.top": False, "axes.spines.right": False, "axes.edgecolor": "#9ca3af",
    "axes.grid": True, "grid.color": "#e5e7eb", "grid.linewidth": 0.8,
    "legend.frameon": False, "svg.fonttype": "path", "svg.hashsalt": "nothinx",
    "lines.linewidth": 1.8,
})
WARNA = {"utama": "#2563eb", "pembanding": "#dc2626", "ketiga": "#16a34a", "keempat": "#9333ea",
         "kelima": "#ea580c", "mentah": "#9ca3af", "target": "#111827"}

DI_SINI = os.path.dirname(os.path.abspath(__file__))
AKAR = os.path.normpath(os.path.join(DI_SINI, "..", ".."))
GAMBAR = os.path.join(AKAR, "extras", "gambar")


def jalankan_simulasi():
    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, "sim")
        src = glob(os.path.join(AKAR, "src", "*.cpp"))
        subprocess.run(["g++", "-std=c++11", "-O2", "-I../test", "-I../../src", "simulasi.cpp", *src,
                        "-o", exe], cwd=DI_SINI, check=True)
        keluaran = subprocess.run([exe], cwd=DI_SINI, check=True, capture_output=True, text=True).stdout
    data = defaultdict(list)
    for baris in keluaran.splitlines():
        bagian, *kolom = baris.split("\t")
        data[bagian].append(kolom)
    return data


def koma(x, digit):
    """Angka untuk teks bahasa Indonesia: 1012.5 -> '1.012,5'."""
    return f"{x:,.{digit}f}".replace(",", "_").replace(".", ",").replace("_", ".")


def kode(s):
    return f"`{s}`"


# Catatan masukan untuk README bahasa Inggris (kolom kedua dari simulasi.cpp)
CATATAN_EN = {
    "akhiran LF": "LF ending", "akhiran CR, huruf campur": "CR ending, mixed case",
    "akhiran CRLF": "CRLF ending", "spasi ganda": "double spaces", "pemisah =": "`=` separator",
    "pemisah koma, negatif": "comma separator, negative", "desimal ditulis dengan koma": "decimal with a comma",
    "bukan angka": "not a number", "baris kosong": "empty line", "70 karakter, buffer 64": "70 chars, 64-byte buffer",
    "baris berikutnya": "next line",
}


def tabel_sesi(d, en):
    kepala = (["Typed", "Note", "`ada()`", "`adalah(\"LED\")`", "Command `teks(0)`", "Arguments `teks(i)`",
               "`angka(i)`", "`desimal(i, NAN)`", "`terlaluPanjang()`"] if en else
              ["Diketik", "Keterangan", "`ada()`", "`adalah(\"LED\")`", "Perintah `teks(0)`", "Argumen `teks(i)`",
               "`angka(i)`", "`desimal(i, NAN)`", "`terlaluPanjang()`"])
    baris = ["| " + " | ".join(kepala) + " |", "|" + "---|" * len(kepala)]
    for dikirim, catatan, ada, panjang, *sisa in d["sesi"]:
        sel = [kode(dikirim), CATATAN_EN[catatan] if en else catatan, ada]
        if ada == "true":
            led, perintah, *arg = sisa
            arg = [a.split("|") for a in arg]
            sel += [led, kode(perintah.split("|")[0]),
                    ", ".join(kode(t) for t, *_ in arg),
                    ", ".join(n if bulat == "1" else "bawaan" if not en else "default" for _, bulat, n, _ in arg),
                    ", ".join(x if x else "NAN" for *_, x in arg)]
        else:
            sel += [""] * 5
        sel.append(panjang)
        baris.append("| " + " | ".join(sel) + " |")
    return "\n".join(baris)


def grafik_waktu(w):
    siap, putaran = float(w["siap_ms"]), int(w["putaran"])
    mulai, selesai = float(w["readstring_mulai_ms"]), float(w["readstring_selesai_ms"])
    akhir = 1100
    fig, ax = plt.subplots(figsize=(8, 2.6))
    tinggi = 0.5
    # PerintahSerial: loop() tidak pernah berhenti
    ax.broken_barh([(0, akhir)], (1 - tinggi / 2, tinggi), color=WARNA["utama"], alpha=0.25, lw=0)
    ax.plot([siap, siap], [1 - tinggi / 2, 1 + tinggi / 2], color=WARNA["utama"], lw=2.5)
    ax.text(akhir / 2, 1, "loop() tetap berputar", color=WARNA["utama"], ha="center", va="center",
            fontweight="bold")
    ax.annotate(f"perintah siap pada {koma(siap, 1)} ms; loop() berputar {putaran}× selama baris diterima",
                (siap, 1 + tinggi / 2), (25, 1 + tinggi / 2 + 0.12), color=WARNA["utama"], va="bottom")
    # readString(): loop() tertahan sampai timeout
    y = 0
    ax.broken_barh([(0, mulai), (selesai, akhir - selesai)], (y - tinggi / 2, tinggi),
                   color=WARNA["utama"], alpha=0.25, lw=0)
    ax.broken_barh([(mulai, selesai - mulai)], (y - tinggi / 2, tinggi), color=WARNA["pembanding"], alpha=0.85, lw=0)
    ax.text((mulai + selesai) / 2, y, "loop() tertahan, menunggu timeout 1 detik", color="white",
            ha="center", va="center", fontweight="bold")
    ax.plot([selesai, selesai], [y - tinggi / 2, y + tinggi / 2], color=WARNA["target"], lw=2.5)
    ax.annotate(f"perintah siap pada {koma(selesai, 1)} ms", (selesai, y + tinggi / 2),
                (selesai, y + tinggi / 2 + 0.12), color=WARNA["pembanding"], va="bottom", ha="right")
    ax.set_yticks([1, 0], ["PerintahSerial\n(ada() di loop)", "Serial.readString()"])
    ax.set_ylim(-0.5, 1.75)
    ax.set_xlim(0, akhir)
    ax.set_xlabel("waktu sejak karakter pertama dikirim (ms)")
    ax.grid(axis="y", visible=False)
    ax.set_axisbelow(True)
    ax.spines["left"].set_visible(False)
    ax.tick_params(axis="y", length=0)
    ax.set_title(f"PerintahSerial siap di {koma(siap, 1)} ms tanpa menahan loop(); "
                 f"readString() baru di {koma(selesai, 1)} ms")
    os.makedirs(GAMBAR, exist_ok=True)
    fig.savefig(os.path.join(GAMBAR, "waktu-baca.svg"), format="svg", metadata={"Date": None})
    plt.close(fig)


def markdown(d, en):
    w = dict(d["waktu"])
    siap, putaran, selesai = float(w["siap_ms"]), w["putaran"], float(w["readstring_selesai_ms"])
    ms_kar, ms_put = float(w["ms_per_karakter"]), float(w["ms_per_putaran"])
    if en:
        return "\n\n".join([
            "**A Serial Monitor session**, run on a PC with a fake `Stream` and this library's code "
            "(`extras/simulasi`). Each row is one line sent; the columns are what the library read. "
            "`angka(i)` shows `default` when the argument is not a whole number.",
            tabel_sesi(d, en=True),
            "![Timing: PerintahSerial keeps loop() running, readString() blocks for one second]"
            "(extras/gambar/waktu-baca.svg)",
            f"Simulation: the line `SET kp 2.5\\r\\n` arrives one character at a time at 9600 baud "
            f"({ms_kar:.2f} ms per character), with an assumed 0.1 ms of other work per `loop()`. With `ada()`, "
            f"the command is ready at {siap:.1f} ms and `loop()` ran {putaran} times meanwhile. "
            f"`Serial.readString()` returns only after the default 1000 ms Stream timeout, at {selesai:.1f} ms, "
            "and `loop()` is stuck until then (computed from Stream.cpp in ArduinoCore-avr 1.8.8, not measured).",
            "Regenerate:\n\n```sh\ncd extras/simulasi\npython gambar.py   # needs g++ and matplotlib\n```",
        ])
    return "\n\n".join([
        "**Sesi Serial Monitor**, dijalankan di PC dengan `Stream` tiruan dan kode library ini "
        "(`extras/simulasi`). Setiap baris tabel adalah satu baris yang dikirim; kolom lainnya adalah apa "
        "yang dibaca library. `angka(i)` bernilai `bawaan` jika argumen bukan bilangan bulat.",
        tabel_sesi(d, en=False),
        "![Diagram waktu: PerintahSerial tidak menahan loop(), readString() menahan 1 detik]"
        "(extras/gambar/waktu-baca.svg)",
        f"Simulasi: baris `SET kp 2.5\\r\\n` datang per karakter pada 9600 baud ({koma(ms_kar, 2)} ms per "
        f"karakter), dengan anggapan isi `loop()` lainnya butuh {koma(ms_put, 1)} ms per putaran. Dengan `ada()`, "
        f"perintah siap pada {koma(siap, 1)} ms dan `loop()` sempat berputar {putaran} kali. "
        f"`Serial.readString()` baru kembali setelah timeout bawaan Stream 1000 ms, yaitu pada "
        f"{koma(selesai, 1)} ms, dan selama itu `loop()` tertahan (dihitung dari Stream.cpp ArduinoCore-avr "
        "1.8.8, bukan diukur).",
        "Tabel dan grafik dibuat dari simulasi di PC yang menjalankan kode library ini "
        "(`extras/simulasi`):\n\n```sh\ncd extras/simulasi\npython gambar.py   # butuh g++ dan matplotlib\n```",
    ])


def sisipkan(nama_file, isi):
    path = os.path.join(AKAR, nama_file)
    with open(path, encoding="utf-8") as f:
        teks = f.read()
    pola = re.compile(r"(<!-- simulasi:awal -->\n).*?(<!-- simulasi:akhir -->)", re.S)
    if not pola.search(teks):
        sys.exit(f"penanda simulasi tidak ada di {nama_file}")
    teks = pola.sub(lambda m: m.group(1) + "\n" + isi + "\n\n" + m.group(2), teks)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(teks)


if __name__ == "__main__":
    d = jalankan_simulasi()
    grafik_waktu(dict(d["waktu"]))
    sisipkan("README.md", markdown(d, en=False))
    sisipkan("README.en.md", markdown(d, en=True))
    print("selesai: README.md, README.en.md, extras/gambar/waktu-baca.svg")
