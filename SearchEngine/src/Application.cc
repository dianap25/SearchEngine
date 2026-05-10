// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Implementacja dyspozytora CLI. Parsuje nazwę polecenia z
// argv, weryfikuje liczbę argumentów i kieruje wywołanie do
// odpowiedniego handlera. Posiada również ekran pomocy wyświetlany,
// gdy program zostanie uruchomiony bez argumentów.

#include "Application.h"

#include "ContextBuilder.h"
#include "Database.h"
#include "IndexService.h"
#include "IndexSummary.h"
#include "RefreshEngine.h"
#include "Repository.h"

#include <cctype>
#include <iostream>
#include <optional>
#include <string>

int Application::run(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "index") {
        if (argc != 3) {
            std::cerr << "Usage: index <directory>\n";
            return 1;
        }

        std::string path = argv[2];
        if (path.empty()) {
            std::cerr << "Empty directory path\n";
            return 1;
        }

        return runIndexCommand(path.c_str());
    }

    if (command == "refresh") {
        if (argc != 3) {
            std::cerr << "Usage: refresh <directory>\n";
            return 1;
        }

        std::string path = argv[2];
        if (path.empty()) {
            std::cerr << "Empty path\n";
            return 1;
        }

        return handleRefresh(path);
    }

    if (command == "search-name") {
        if (argc != 3) {
            std::cerr << "Usage: search-name <phrase>\n";
            return 1;
        }

        std::string phrase = argv[2];
        if (phrase.empty()) {
            std::cerr << "Empty search phrase\n";
            return 1;
        }

        return handleSearchName(phrase);
    }

    if (command == "search-content") {
        if (argc != 3) {
            std::cerr << "Usage: search-content <word>\n";
            return 1;
        }

        std::string word = argv[2];
        if (word.empty()) {
            std::cerr << "Empty search word\n";
            return 1;
        }

        return handleSearchContent(word);
    }

    std::cerr << "Unknown command: " << command << "\n";
    printUsage();
    return 1;
}

int Application::runIndexCommand(const char* directory_path) {
    Database database;

    if (!database.open("index.db")) {
        std::cerr << "Cannot open database\n";
        return 1;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Cannot initialize database schema\n";
        return 1;
    }

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(directory_path);

    std::cout << "Indexing finished\n";
    std::cout << "Scanned files: " << summary.scanned_files << "\n";
    std::cout << "Indexed files: " << summary.indexed_files << "\n";
    std::cout << "Skipped files: " << summary.skipped_files << "\n";
    std::cout << "Failed files:  " << summary.failed_files << "\n";

    return summary.failed_files == 0 ? 0 : 2;
}

int Application::handleRefresh(const std::string& path) {
    Database db;
    if (!db.open("index.db")) return 1;

    if (!db.initializeSchema()) {
        std::cerr << "Cannot initialize database schema\n";
        return 1;
    }

    RefreshEngine engine;
    engine.refresh(path, db);

    std::cout << "Refresh completed\n";
    return 0;
}

int Application::handleSearchName(const std::string& phrase) {
    Database db;

    if (!db.open("index.db")) {
        std::cerr << "Cannot open database\n";
        return 1;
    }

    Repository repo(db.connection());

    auto results = repo.searchByName(phrase);

    if (results.empty()) {
        std::cout << "No results\n";
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
        std::cerr << "Cannot open database\n";
        return 1;
    }

    Repository repo(db.connection());

    std::string normalized = normalize(word);

    auto results = repo.searchByContent(normalized);

    if (results.empty()) {
        std::cout << "No matches\n";
        return 0;
    }

    ContextBuilder context_builder(40);

    for (const auto& r : results) {
        std::cout << r.path
                  << " -> occurrences: " << r.occurrences
                  << std::endl;

        std::optional<std::string> text = repo.findTextByPath(r.path);

        if (text.has_value()) {
            std::string context = context_builder.build(text.value(), word);

            if (!context.empty()) {
                std::cout << "  Context: " << context << "\n";
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
    std::cout << "  SearchEngine - local file search\n";
    std::cout << "==============================================\n\n";

    std::cout << "What now? Run the steps below in order:\n\n";

    std::cout << "  1) Build the index for a directory (once, or after new files):\n";
    std::cout << "       ./searchengine index ../examples\n\n";

    std::cout << "  2) Search files by a fragment of their name:\n";
    std::cout << "       ./searchengine search-name kawa\n\n";

    std::cout << "  3) Search by a word inside file contents (with a context snippet):\n";
    std::cout << "       ./searchengine search-content tatry\n\n";

    std::cout << "  4) After files change on disk, refresh the index:\n";
    std::cout << "       ./searchengine refresh ../examples\n\n";

    std::cout << "Full command reference:\n";
    std::cout << "  ./searchengine index <dir>             - build the index\n";
    std::cout << "  ./searchengine refresh <dir>           - update the index\n";
    std::cout << "  ./searchengine search-name <phrase>    - search by file name\n";
    std::cout << "  ./searchengine search-content <word>   - search inside file contents\n\n";

    std::cout << "Tip: the 'examples/' folder ships with ready-to-index sample files.\n";
}
