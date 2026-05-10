# SearchEngine - Instrukcja kompilacji i uruchomienia

Projekt wyszukiwarki napisany w C++ z wykorzystaniem bibliotek SQLite3, OpenSSL oraz GoogleTest.

## Wymagania systemowe

Do poprawnej kompilacji projektu wymagane są:
* **CMake** (wersja 3.16 lub nowsza)
* **Kompilator C++17** (g++ lub Apple Clang)
* **SQLite3**
* **OpenSSL**
* **GoogleTest (GTest)**
* **Poppler-utils (dla pdftotext)**

### Instalacja zależności (Linux / Debian / Ubuntu)

```bash
sudo apt update
sudo apt install cmake g++ libsqlite3-dev libssl-dev libgtest-dev poppler-utils
```

### Instalacja zależności (macOS / Homebrew)

```bash
brew install cmake sqlite openssl googletest poppler
```

## Plik wejściowy projektu

Punktem wejścia aplikacji jest plik [src/main.cpp](src/main.cpp). To w nim tworzona jest instancja klasy `Application` i wywoływana metoda `run()`. Cała logika obsługi poleceń znajduje się w [src/Application.cpp](src/Application.cpp).

Po skompilowaniu projektu otrzymujemy plik wykonywalny `searchengine` w katalogu `build/`. To jego uruchamiamy z linii poleceń, podając odpowiednie polecenie jako argument.

## Kompilacja i uruchomienie

```bash
mkdir build
cd build
cmake ..
make
./searchengine
```

Uruchomienie programu bez argumentów spowoduje wyświetlenie powitania oraz pełnej instrukcji dla użytkownika 

## Sposób użycia

Program obsługuje cztery polecenia. Wszystkie uruchamiane są z katalogu `build/`.

### 1. Indeksowanie katalogu

Najpierw należy zbudować indeks dla wybranego folderu z plikami. Indeks zapisywany jest w pliku `index.db` w bieżącym katalogu.

```bash
./searchengine index ../examples
```

W repozytorium znajduje się gotowy folder [examples/](examples/), zawierający kilka plików tekstowych 

### 2. Wyszukiwanie po nazwie pliku

```bash
./searchengine search-name kawa
```

Program zwróci ścieżki wszystkich plików, których nazwa zawiera podaną frazę (porównanie jest niezależne od wielkości liter).

### 3. Wyszukiwanie po treści pliku

```bash
./searchengine search-content tatry
```

Program przeszuka indeks pełnotekstowy i wyświetli pliki zawierające podane słowo, posortowane malejąco po liczbie wystąpień. Dla każdego trafienia drukowany jest dodatkowo krótki **kontekst** – fragment tekstu wokół znalezionej frazy (domyślnie ±40 znaków)

Przykładowy wynik:

```
../examples/wyprawa_w_tatry.txt -> occurrences: 2
  Context: ranem wyruszylismy z Zakopanego w kierunku Doliny Piecu Stawow Polskich...
```

### 4. Odświeżanie indeksu

Po dodaniu, usunięciu lub zmodyfikowaniu plików wystarczy uruchomić:

```bash
./searchengine refresh ../examples
```

Indeks zostanie zaktualizowany inkrementalnie – pliki niezmienione (rozpoznawane po skrócie SHA-256) zostaną pominięte.

## Typowy przepływ pracy

```bash
cd build
./searchengine index ../examples            # 1. zbuduj indeks
./searchengine search-name nalesniki        # 2. szukaj po nazwie
./searchengine search-content programowanie # 3. szukaj po tresci
./searchengine refresh ../examples          # 4. odswiez po zmianach
```

## Uruchomienie testów

```bash
cd build
ctest --output-on-failure
```

Lub bezpośrednio:

```bash
./tests/unit_tests
```

## Generowanie dokumentacji

Cały kod ma komentarze w formacie Doxygen (bloki `@brief`, `@param`, `@return` przy klasach i metodach publicznych), co pozwala wygenerowac dokumentację HTML

### Wymagania

* **Doxygen** (≥ 1.9)
* **Graphviz** — opcjonalny, do rysowania diagramów dziedziczenia i zależności (`HAVE_DOT = YES` w `Doxyfile`)

#### Instalacja zależności

Linux (Debian / Ubuntu):

```bash
sudo apt install doxygen graphviz
```

macOS (Homebrew):

```bash
brew install doxygen graphviz
```


### Generowanie

W katalogu `SearchEngine/` znajduje się gotowy plik konfiguracyjny [Doxyfile](Doxyfile). Aby wygenerować dokumentację, wystarczy uruchomić:

```bash
cd SearchEngine
doxygen Doxyfile
```

Wynik zostanie zapisany w katalogu `docs/html/`. Stronę startową otwieramy w przeglądarce:

```bash
open docs/html/index.html      # macOS
xdg-open docs/html/index.html  # Linux
```


