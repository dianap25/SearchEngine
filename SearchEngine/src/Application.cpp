//Alesia Filinkova
//Diana Pelin

#include "Application.h"

#include "Database.h"
#include "IndexService.h"
#include "IndexSummary.h"

#include <iostream>
#include <string>

int Application::run(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "index") {
        if (argc != 3) {
            std::cerr << "Missing directory path for index command\n";
            printUsage();
            return 1;
        }

        return runIndexCommand(argv[2]);
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

void Application::printUsage() const {
    std::cout << "Usage:\n";
    std::cout << "  ./searchengine index <directory>\n";
}