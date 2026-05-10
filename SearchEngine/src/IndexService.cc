// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of IndexService, which scans a
// directory, extracts text per file, hashes it and persists the
// metadata, full text and inverted-index postings under a
// per-file SQLite transaction.


#include "IndexService.h"

#include "ExtractResult.h"
#include "Extractor.h"
#include "ExtractorFactory.h"
#include "Hasher.h"
#include "Indexer.h"
#include "Repository.h"
#include "Scanner.h"

#include <iostream>
#include <memory>
#include <vector>

IndexService::IndexService(Database& database)
    : database_(database) {
}

IndexSummary IndexService::indexDirectory(const std::string& root_path) {
    IndexSummary summary;

    Scanner scanner;
    Repository repository(database_.connection());
    Indexer indexer(repository);

    std::vector<FileMetadata> files = scanner.scan(root_path);
    summary.scanned_files = static_cast<int>(files.size());

    for (const FileMetadata& file : files) {
        std::unique_ptr<Extractor> extractor = ExtractorFactory::create(file.path);
        ExtractResult result = extractor->extract(file.path);
        if (!result.success) continue;
        std::string content = result.content;

        if (content.empty()) {
            std::cerr << "Pominieto pusty lub nieczytelny plik: " << file.path << "\n";
            ++summary.skipped_files;
            continue;
        }

        if (!repository.beginTransaction()) {
            std::cerr << "Nie udalo sie rozpoczac transakcji dla pliku: " << file.path << "\n";
            ++summary.failed_files;
            continue;
        }

        FileMetadata file_with_hash = file;
        file_with_hash.content_hash = Hasher::sha256(content);

        int file_id = repository.saveFileMetadata(file_with_hash);

        if (file_id < 0) {
            std::cerr << "Nie udalo sie zapisac metadanych pliku: " << file.path << "\n";
            repository.rollbackTransaction();
            ++summary.failed_files;
            continue;
        }

        if (!repository.saveFileText(file_id, content)) {
            std::cerr << "Nie udalo sie zapisac tresci pliku: " << file.path << "\n";
            repository.rollbackTransaction();
            ++summary.failed_files;
            continue;
        }

        if (!indexer.indexFile(file_id, content)) {
            std::cerr << "Nie udalo sie zaindeksowac pliku: " << file.path << "\n";
            repository.rollbackTransaction();
            ++summary.failed_files;
            continue;
        }

        if (!repository.commitTransaction()) {
            std::cerr << "Nie udalo sie zatwierdzic transakcji dla pliku: " << file.path << "\n";
            repository.rollbackTransaction();
            ++summary.failed_files;
            continue;
        }

        ++summary.indexed_files;
    }

    return summary;
}
