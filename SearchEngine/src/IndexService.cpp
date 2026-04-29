//Alesia Filinkova
//Diana Pelin

#include "IndexService.h"

#include "Extractor.h"
#include "Indexer.h"
#include "Repository.h"
#include "Scanner.h"

#include <iostream>
#include <vector>

IndexService::IndexService(Database& database)
    : database_(database) {
}

IndexSummary IndexService::indexDirectory(const std::string& rootPath) {
    IndexSummary summary;

    Scanner scanner;
    Extractor extractor;
    Repository repository(database_.connection());
    Indexer indexer(repository);

    std::vector<FileMetadata> files = scanner.scan(rootPath);
    summary.scannedFiles = static_cast<int>(files.size());

    for (const FileMetadata& file : files) {
        std::string content = extractor.extract(file.path);

        if (content.empty()) {
            std::cerr << "Skipped empty or unreadable file: " << file.path << "\n";
            summary.skippedFiles++;
            continue;
        }

        if (!repository.beginTransaction()) {
            std::cerr << "Failed to start transaction for file: " << file.path << "\n";
            summary.failedFiles++;
            continue;
        }

        int fileId = repository.saveFileMetadata(file);

        if (fileId < 0) {
            std::cerr << "Failed to save file metadata: " << file.path << "\n";
            repository.rollbackTransaction();
            summary.failedFiles++;
            continue;
        }

        if (!repository.saveFileText(fileId, content)) {
            std::cerr << "Failed to save file text: " << file.path << "\n";
            repository.rollbackTransaction();
            summary.failedFiles++;
            continue;
        }

        if (!indexer.indexFile(fileId, content)) {
            std::cerr << "Failed to index file: " << file.path << "\n";
            repository.rollbackTransaction();
            summary.failedFiles++;
            continue;
        }

        if (!repository.commitTransaction()) {
            std::cerr << "Failed to commit transaction for file: " << file.path << "\n";
            repository.rollbackTransaction();
            summary.failedFiles++;
            continue;
        }

        summary.indexedFiles++;
    }

    return summary;
}