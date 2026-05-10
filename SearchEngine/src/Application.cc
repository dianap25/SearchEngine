// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of the CLI dispatcher for the
// SearchEngine application (index, refresh, search-name,
// search-content).

#include "Application.h"

#include "ContextBuilder.h"
#include "Database.h"
#include "IndexService.h"
#include "IndexSummary.h"
#include "RefreshEngine.h"
#include "Repository.h"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>

namespace {

// Number of characters before and after each match included in the
// context snippet printed by the search-content command.
constexpr std::size_t CONTEXT_MARGIN = 40;

// Process exit code returned when the indexing run completed but at
// least one file failed to be indexed.
constexpr int EXIT_PARTIAL_FAILURE = 2;

} // namespace

int Application::run(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "index") {
        if (argc != 3) {
            std::cerr << "Uzycie: index <katalog>\n";
            return 1;
        }

        std::string path = argv[2];
        if (path.empty()) {
            std::cerr << "Pusta sciezka katalogu\n";
            return 1;
        }

        return runIndexCommand(path.c_str());
    }

    if (command == "refresh") {
        if (argc != 3) {
            std::cerr << "Uzycie: refresh <katalog>\n";
            return 1;
        }

        std::string path = argv[2];
        if (path.empty()) {
            std::cerr << "Pusta sciezka\n";
            return 1;
        }

        return handleRefresh(path);
    }

    if (command == "search-name") {
        if (argc != 3) {
            std::cerr << "Uzycie: search-name <fraza>\n";
            return 1;
        }

        std::string phrase = argv[2];
        if (phrase.empty()) {
            std::cerr << "Pusta fraza wyszukiwania\n";
            return 1;
        }

        return handleSearchName(phrase);
    }

    if (command == "search-content") {
        if (argc != 3) {
            std::cerr << "Uzycie: search-content <slowo>\n";
            return 1;
        }

        std::string word = argv[2];
        if (word.empty()) {
            std::cerr << "Puste slowo wyszukiwania\n";
            return 1;
        }

        return handleSearchContent(word);
    }

    std::cerr << "Nieznane polecenie: " << command << "\n";
    printUsage();
    return 1;
}

int Application::runIndexCommand(const char* directory_path) {
    Database database;

    if (!database.open("index.db")) {
        std::cerr << "Nie mozna otworzyc bazy danych\n";
        return 1;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Nie mozna zainicjalizowac schematu bazy\n";
        return 1;
    }

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(directory_path);

    std::cout << "Indeksowanie zakonczone\n";
    std::cout << "Pliki przeskanowane: " << summary.scanned_files << "\n";
    std::cout << "Pliki zaindeksowane: " << summary.indexed_files << "\n";
    std::cout << "Pliki pominiete:     " << summary.skipped_files << "\n";
    std::cout << "Pliki z bledami:     " << summary.failed_files << "\n";

    return summary.failed_files == 0 ? 0 : EXIT_PARTIAL_FAILURE;
}

int Application::handleRefresh(const std::string& path) {
    Database db;
    if (!db.open("index.db")) return 1;

    if (!db.initializeSchema()) {
        std::cerr << "Nie mozna zainicjalizowac schematu bazy\n";
        return 1;
    }

    RefreshEngine engine;
    engine.refresh(path, db);

    std::cout << "Odswiezanie zakonczone\n";
    return 0;
}

int Application::handleSearchName(const std::string& phrase) {
    Database db;

    if (!db.open("index.db")) {
        std::cerr << "Nie mozna otworzyc bazy danych\n";
        return 1;
    }

    Repository repo(db.connection());

    auto results = repo.searchByName(phrase);

    if (results.empty()) {
        std::cout << "Brak wynikow\n";
        return 0;
    }

    for (const auto& r : results) {
        std::cout << r.path << std::endl;
    }

    return 0;
}

int Application::handleSearchContent(const std::string& word) {
    Database db;

    if (!db.open("index.db")) {
        std::cerr << "Nie mozna otworzyc bazy danych\n";
        return 1;
    }

    Repository repo(db.connection());

    std::string normalized = normalize(word);

    auto results = repo.searchByContent(normalized);

    if (results.empty()) {
        std::cout << "Brak trafien\n";
        return 0;
    }

    ContextBuilder context_builder(CONTEXT_MARGIN);

    for (const auto& r : results) {
        std::cout << r.path
                  << " -> wystapien: " << r.occurrences
                  << std::endl;

        std::optional<std::string> text = repo.findTextByPath(r.path);

        if (text.has_value()) {
            std::string context = context_builder.build(text.value(), word);

            if (!context.empty()) {
                std::cout << "  Kontekst: " << context << "\n";
            }
        }
    }

    return 0;
}

std::string Application::normalize(const std::string& input) {
    std::string result;

    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result += std::tolower(c);
        }
    }

    return result;
}

void Application::printUsage() const {
    std::cout << "==============================================\n";
    std::cout << "  SearchEngine - lokalna wyszukiwarka plikow\n";
    std::cout << "==============================================\n\n";

    std::cout << "Co teraz zrobic? Wykonaj kroki w tej kolejnosci:\n\n";

    std::cout << "  1) Zbuduj indeks dla katalogu (jednorazowo lub po nowych plikach):\n";
    std::cout << "       ./searchengine index ../examples\n\n";

    std::cout << "  2) Szukaj plikow po fragmencie nazwy:\n";
    std::cout << "       ./searchengine search-name kawa\n\n";

    std::cout << "  3) Szukaj po slowie w tresci plikow (z fragmentem kontekstu):\n";
    std::cout << "       ./searchengine search-content tatry\n\n";

    std::cout << "  4) Po zmianach plikow na dysku odswiez indeks:\n";
    std::cout << "       ./searchengine refresh ../examples\n\n";

    std::cout << "Pelna lista polecen:\n";
    std::cout << "  ./searchengine index <katalog>          - buduje indeks\n";
    std::cout << "  ./searchengine refresh <katalog>        - aktualizuje indeks\n";
    std::cout << "  ./searchengine search-name <fraza>      - szuka po nazwie pliku\n";
    std::cout << "  ./searchengine search-content <slowo>   - szuka w tresci pliku\n\n";

    std::cout << "Wskazowka: katalog 'examples/' zawiera gotowe pliki testowe.\n";
}
