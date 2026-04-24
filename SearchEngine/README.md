# SearchEngine - Instrukcja kompilacji i uruchomienia

Projekt wyszukiwarki napisany w C++ z wykorzystaniem bibliotek SQLite3 oraz GoogleTest.

##  Wymagania systemowe

Do poprawnej kompilacji projektu wymagane są:
* **CMake** (wersja 3.10 lub nowsza)
* **Kompilator C++** (g++ lub Apple Clang)
* **SQLite3**
* **GoogleTest (GTest)**
* **Poppler-utils (for pdftotext)**


### Instalacja zależności

```bash
sudo apt update
sudo apt install cmake g++ libsqlite3-dev libgtest-dev poppler-utils
```



### Uruchomienie aplikacji

```bash
mkdir build
cd build
cmake ..
make
./searchengine
```


### Uruchomienie testów

```bash
ctest --output-on-failure
```