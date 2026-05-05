# Procedurális Terepgenerátor OpenCL és OpenGL Interoperabilitással

Beadandó projekt a **Párhuzamos eszközök programozása** (GEMAK259-B) kurzushoz.
Készítette: Tóth Zsombor Gábor, D0H157

## A projektről

Ez a projekt a **Fraktális Brown-mozgás (fBm)** és a **Perlin-zaj** algoritmusok GPU-alapú, párhuzamosított megvalósítása OpenCL segítségével. A szoftver valós időben, memóriafoglalás (zero memory footprint) nélkül generál matematikailag végtelen méretű tereptérképeket.

A projekt két fő komponensből áll:
1. **Interaktív térkép (`map.exe`)**: Valós idejű, WASD gombokkal navigálható 2D tereptérkép. Az OpenCL-OpenGL megosztott kontextusnak (Interop) köszönhetően a renderelés "zero-copy" módon történik, másodpercenként 60+ FPS-el.
2. **Teljesítménymérő (`benchmark.exe`)**: Referencia program, amely memóriapuffereket használva méri a kernel lefutási idejét különböző felbontásokon (egészen 4K-ig), és az eredményeket `.ppm` képfájlokként menti el.

## Főbb funkciók

* **Optimalizált Perlin-zaj**: Átgondolt PCG (Permuted Congruential Generator) hash függvény és 12 irányú gradiens-készlet a rács-műtermékek (lattice artifacts) elkerülése érdekében.
* **FBM (Fractal Brownian Motion)**: 4 és 6 oktávos zajrétegzés a részletgazdag, természetes formák (felhők, hegyek, óceánok) eléréséhez.
* **OpenGL Interoperabilitás**: A `cl_khr_gl_sharing` kiterjesztés használata. Az OpenCL kernel közvetlenül a videókártya textúra-memóriájába (`image2d_t`) ír, kikerülve a CPU-GPU sávszélesség-szűkületet.
* **Natív Windows API (Win32)**: Független külső ablakkezelő könyvtáraktól (pl. GLFW, SDL), tisztán Win32 API-t és WGL-t használ.

## Fordítás és Futtatás

### Rendszerkövetelmények
* Windows Operációs Rendszer
* MinGW GCC fordító: [Piller Imre tanár úr fordítója](https://web.uni-miskolc.hu/~matip/_downloads/c_sdk_220203.zip)
* OpenCL SDK (a környezeti változókban beállítva)

### Fordítás
A mellékelt `Makefile` segítségével mindkét program egy lépésben lefordítható.
```bash
make
```

### Futtatás
**1. Az Interaktív Terepgenerátor futtatása:**
```bash
./map.exe
```
* **Irányítás:** `W`, `A`, `S`, `D` billentyűk a kamera mozgatásához.
* **Monitorozás:** Az ablak címsorában valós időben látható a számított FPS és a GPU kernel futási ideje (ms).

**2. A Sebességmérő (Benchmark) futtatása:**
```bash
./benchmark.exe
```
* Lefuttatja a zajgenerálást több különböző felbontáson (pl. 1920x1080, 4K).
* A konzolra kiírja a kernel végrehajtási idejét.
* A futás végén a generált textúrákat `.ppm` (Portable PixMap) formátumban elmenti a könyvtárba.

## Teljesítmény

A részletes elemzés és a fejlesztés folyamata a dokumentációban olvasható. A tesztek igazolták a GPU-alapú megközelítés hatékonyságát:

* Egy 1024x1024-es, 6 oktávos textúra legenerálása natív, szekvenciális **C kóddal CPU-n ~300-400 ms**-ot vesz igénybe.
* Ugyanez a feladat a megfelelően hangolt (16x16 Local Work Size) **OpenCL kernellel GPU-n csupán ~3-4 ms** alatt lefut, megnyitva az utat a 60+ FPS-es valós idejű animáció előtt.

## Dokumentáció

Mellékelve van egy-egy `Procedurális textúragenerálás OpenCL és OpenGL segítségével` markdown, valamint PDF fájl a projekt részletes matematikai és hardveres elemzésével.

> A markdown fájl **Obsidian**-ban lett megírva, a PDF formátum pedig **Pandoc export as LaTeX** segítségével lett kigenerálva.