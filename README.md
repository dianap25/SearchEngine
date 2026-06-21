# SearchEngine

A local file search engine written in C++17. It indexes a directory of documents
(`.txt`, `.tex`, `.pdf`) into a SQLite database and lets you search them from the
command line — by file name or by full-text content, with a short context snippet
printed around each match. After files change on disk, the index can be refreshed
incrementally: unchanged files (recognised by their SHA-256 hash) are skipped.

## Features

- **Indexing** — recursively scans a directory and stores file metadata and
  extracted text in a local SQLite index (`index.db`).
- **Search by name** — returns every indexed file whose name contains the given
  phrase (case-insensitive).
- **Search by content** — full-text search ranked by number of occurrences, with a
  context snippet (±40 characters) printed around each hit.
- **Incremental refresh** — re-scans a directory and updates only files that were
  added, removed or modified, detected via SHA-256 hashing.
- **Multiple formats** — plain text and LaTeX are read directly; PDF text is
  extracted via `pdftotext` (poppler-utils).

## Requirements

| Dependency | Purpose | Ubuntu package |
| --- | --- | --- |
| CMake ≥ 3.16 | build system | `cmake` |
| C++17 compiler (g++) | compilation | `g++` |
| SQLite3 | index storage and full-text search | `libsqlite3-dev` |
| OpenSSL | SHA-256 hashing for change detection | `libssl-dev` |
| GoogleTest | unit tests | `libgtest-dev` |
| poppler-utils | PDF text extraction (`pdftotext`) | `poppler-utils` |

The project targets **Ubuntu 24.04 LTS** and uses only libraries available in the
distribution's standard packages. It also builds on macOS with Apple Clang.

### Installing dependencies

Linux (Debian / Ubuntu):

```bash
sudo apt update
sudo apt install cmake g++ libsqlite3-dev libssl-dev libgtest-dev poppler-utils
```

macOS (Homebrew):

```bash
brew install cmake sqlite openssl googletest poppler
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

The build produces the executable `searchengine` in the `build/` directory.

## Usage

All commands are run from the `build/` directory. The index is stored in
`index.db` in the current directory. Running the program with no arguments prints a
short usage guide.

```bash
./searchengine index <directory>          # build the index for a directory
./searchengine search-name <phrase>       # search by file name
./searchengine search-content <word>      # search by file content
./searchengine refresh <directory>        # incrementally update the index
```

A ready-made `examples/` directory with sample documents is included.

### Typical workflow

```bash
cd build
./searchengine index ../examples            # 1. build the index
./searchengine search-name nalesniki        # 2. search by name
./searchengine search-content programowania # 3. search by content
./searchengine refresh ../examples          # 4. refresh after changes
```

Example output of a content search:

```
../examples/wyprawa_w_tatry.txt -> wystapien: 2
  Kontekst: ranem wyruszylismy z Zakopanego w kierunku Doliny Pieciu Stawow Polskich...
```

## Running the tests

The project ships with GoogleTest-based unit tests covering every module
(11 test files). From the `build/` directory:

```bash
ctest --output-on-failure
```

or run the test executable directly:

```bash
./tests/unit_tests
```

## Architecture

The sources are split into small, single-responsibility modules under `include/`
and `src/`:

- **Application** — CLI dispatcher; parses arguments and routes each command to the
  relevant service.
- **Scanner** — walks the target directory and yields file paths and metadata.
- **Extractor / TextExtractor / PdfExtractor / ExtractorFactory** — an extraction
  strategy hierarchy (Strategy pattern); the factory selects the right extractor
  from the file extension. PDFs go to `PdfExtractor` (poppler), everything else to
  `TextExtractor`.
- **Hasher** — computes SHA-256 digests (OpenSSL) used to detect changed files.
- **Database** — RAII wrapper around the SQLite connection (`std::unique_ptr` with a
  custom deleter); creates and migrates the schema.
- **Repository** — query layer over SQLite (search by name, search by content,
  fetch stored text).
- **Indexer / IndexService** — orchestrate scanning, extraction and storage; report
  an `IndexSummary` (scanned / indexed / skipped / failed counts).
- **RefreshEngine** — performs incremental updates against an existing index.
- **ContextBuilder** — builds the context snippet shown around a content match.

## Documentation

The code is annotated with Doxygen comments (`@brief`, `@param`, `@return`) on
public classes and methods. With a `Doxyfile` present, HTML documentation can be
generated with:

```bash
doxygen Doxyfile     # output in docs/html/
```

Doxygen and Graphviz are required (`sudo apt install doxygen graphviz`).

## Authors

Alesia Filinkova, Diana Pelin
