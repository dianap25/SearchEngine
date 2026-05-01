//Alesia Filinkova
//Diana Pelin

#include "Application.h"

#include "Database.h"
#include "IndexService.h"
#include "IndexSummary.h"
#include "RefreshEngine.h"
#include "Repository.h"

#include <iostream>
#include <string>
#include <cctype>

int Application::run(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "No command provided\n";
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

int Application::runIndexCommand(const char* directoryPath) {
    Database database;

    if (!database.open("index.db")) {
        std::cerr << "Cannot open database\n";
        return 1;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Cannot initialize database schema\n";
        return 1;
    }

    IndexService indexService(database);
    IndexSummary summary = indexService.indexDirectory(directoryPath);

    std::cout << "Indexing finished\n";
    std::cout << "Scanned files: " << summary.scannedFiles << "\n";
    std::cout << "Indexed files: " << summary.indexedFiles << "\n";
    std::cout << "Skipped files: " << summary.skippedFiles << "\n";
    std::cout << "Failed files: " << summary.failedFiles << "\n";

    return summary.failedFiles == 0 ? 0 : 2;
}

int Application::handleRefresh(const std::string& path) {
    Database db;
    if (!db.open("index.db")) return 1;

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

    for (const auto& r : results) {
        std::cout << r.path 
                  << " -> occurrences: " << r.occurrences 
                  << std::endl;
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
    std::cout << "Usage:\n";
    std::cout << "  ./searchengine index <directory>\n";
    std::cout << "  ./searchengine refresh <directory>\n";
    std::cout << "  ./searchengine search-name <phrase>\n";
    std::cout << "  ./searchengine search-content <word>\n";
}