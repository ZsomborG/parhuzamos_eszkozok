## Feladatok Leírása és Használat

### 1. Eszközinformációk (`clinfo`)
Lekérdezi a rendszerben elérhető OpenCL platformokat és eszközöket (GPU/CPU), majd kiírja azok tulajdonságait (memória mérete, számítóegységek száma, stb.).

*   **Forráskód:** `clinfo.c`
*   **Fordítás:**
    ```bash
    gcc clinfo.c -o clinfo -lOpenCL
    ```
*   **Futtatás:** `.\clinfo.exe`

### 2. Kódbetöltő (Implementálva a `kernel_loader`-ben)
Ez nem egy önálló feladat, hanem egy segédmodul (`kernel_loader.c`), amely lehetővé teszi, hogy a C programok külső szöveges fájlokból (`.cl`) olvassák be a GPU kódot. Minden további feladat ezt használja.

### 3. Leképzések / Mapping (`mappings`)
Bemutatja az index-manipuláció alapjait. Egyetlen C program futtat le egymás után három különböző kernelt ugyanazon az adatsoron.
1.  **Identity:** Eredeti indexek visszaírása.
2.  **Reverse:** Tömb megfordítása.
3.  **Swap:** Szomszédos páros és páratlan elemek felcserélése.

*   **Forráskód:** `mappings.c`
*   **Kernel:** `kernels/mapping.cl`
*   **Fordítás:**
    ```bash
    gcc mappings.c kernel_loader.c -o mappings -lOpenCL
    ```
*   **Futtatás:** `.\mappings.exe`

### 4. Vektorok összeadása (`vector_add`)
Két lebegőpontos (float) vektor összeadása párhuzamosan. A program elrejti az OpenCL hívásokat egy függvénybe, és a végén a CPU-n is kiszámolja az eredményt ellenőrzésképpen.

*   **Forráskód:** `vector_add.c`
*   **Kernel:** `kernels/vector_add.cl`
*   **Fordítás:**
    ```bash
    gcc vector_add.c kernel_loader.c -o vector_add -lOpenCL
    ```
*   **Futtatás:** `.\vector_add.exe`

### 5. Hiányzó elemek pótlása (`fill_missing`)
Adat-előfeldolgozási feladat. A program generál egy számsorozatot, amelyből véletlenszerűen töröl elemeket (ezeket `_` jelzi). A GPU kernel párhuzamosan állítja vissza a hiányzó adatokat a szomszédok átlaga alapján.

*   **Forráskód:** `fill_missing.c`
*   **Kernel:** `kernels/fill_missing.cl`
*   **Fordítás:**
    ```bash
    gcc fill_missing.c kernel_loader.c -o fill_missing -lOpenCL
    ```
*   **Futtatás:** `.\fill_missing.exe`

### 6. Rangszámítás (`rank`)
Minden elemre meghatározza, hány nála szigorúan kisebb elem található a tömbben. Ez egy $O(N^2)$ bonyolultságú feladat, amit a GPU-n $N$ szálon párhuzamosítunk (minden szál végignézi a tömböt a saját eleméhez).

*   **Forráskód:** `rank.c`
*   **Kernel:** `kernels/rank.cl`
*   **Fordítás:**
    ```bash
    gcc rank.c kernel_loader.c -o rank -lOpenCL
    ```
*   **Futtatás:** `.\rank.exe`

### 7. Elemek előfordulásának száma és egyediség (`count_occurrences`)
Párhuzamosan megszámolja egy tömb minden elemének előfordulását (hányszor szerepel a tömbben). A Host kód ezután kiértékeli a GPU eredményét, és eldönti, hogy a tömb minden eleme egyedi-e (nincs-e duplikátum).

*   **Forráskód:** `count_occurrences.c`
*   **Kernel:** `kernels/count_occurrences.cl`
*   **Fordítás:**
    ```bash
    gcc count_occurrences.c kernel_loader.c -o count_occurrences -lOpenCL
    ```
*   **Futtatás:** `.\count_occurrences.exe`

### 8. Szélsőérték vizsgálat konstans időben (`extrema`)
PRAM CRCW modell alapján $O(1)$ időben határozza meg egy tömb maximális elemét. A felhasznált magok számát optimalizálja azzal, hogy a kétdimenziós indexmátrixnak csak a felső háromszögét vizsgálja meg (kizárva a felesleges ismételt vizsgálatokat).

*   **Forráskód:** `extrema.c`
*   **Kernel:** `kernels/extrema.cl`
*   **Fordítás:**
    ```bash
    gcc extrema.c kernel_loader.c -o extrema -lOpenCL
    ```
*   **Futtatás:** `.\extrema.exe`

### 9. Csúszóátlag számítása (`moving_average`)
Jelfeldolgozási feladat, amely egy megadott sugarú (radius) környezeten belüli elemek átlagát számítja ki minden ponthoz, ezáltal zajszűrést (smoothing) végezve az adatsoron. Helyesen kezeli a tömb szélein fellépő csonkított ablakméreteket.

*   **Forráskód:** `moving_average.c`
*   **Kernel:** `kernels/moving_average.cl`
*   **Fordítás:**
    ```bash
    gcc moving_average.c kernel_loader.c -o moving_average -lOpenCL
    ```
*   **Futtatás:** `.\moving_average.exe`

### 10. Prím vizsgálat (`prime_check`)
Prímszámtesztelés három különböző párhuzamosítási stratégiával (egy osztó/szál, osztótartomány/szál, előre generált prímek/szál). A CPU Eratoszthenész szitájával generálja le a prímeket $\sqrt{N}$-ig, majd ezeket betölti a memóriába a kernel 3. módjának futtatásához.

*   **Forráskód:** `prime_check.c`
*   **Kernel:** `kernels/prime_check.cl`
*   **Fordítás:**
    ```bash
    gcc prime_check.c kernel_loader.c -o prime_check -lOpenCL
    ```
*   **Futtatás:** `.\prime_check.exe`