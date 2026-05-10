// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of RefreshEngine. Compares filesystem
// state with the database (matched by SHA-256 of content) and
// inserts, re-indexes or removes records to keep them in sync.


#include "RefreshEngine.h"

#include "ExtractResult.h"
#include "Extractor.h"
#include "ExtractorFactory.h"
#include "Hasher.h"
#include "Indexer.h"
#include "Repository.h"
#include "Scanner.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

void RefreshEngine::refresh(const std::string& root_path, Database& db) {
    if (!db.initializeSchema()) {
        std::cerr << "Nie udalo sie zainicjalizowac schematu przed odswiezeniem\n";
        return;
    }

    Scanner scanner;

    Repository repo(db.connection());
    Indexer indexer(repo);

    repo.beginTransaction();

    auto fs_files = scanner.scan(root_path);
    auto db_files = repo.findAllFiles();

    std::unordered_map<std::string, FileMetadata> db_map;
    for (const auto& f : db_files) {
        db_map[f.path] = f;
    }

    std::unordered_set<std::string> seen;

    for (const auto& fs_file : fs_files) {
        seen.insert(fs_file.path);
        std::unique_ptr<Extractor> extractor = ExtractorFactory::create(fs_file.path);
        ExtractResult result = extractor->extract(fs_file.path);
        if (!result.success) {
            std::cerr << "[POMINIETO] " << fs_file.path
                      << " -> " << result.error_message << "\n";
            continue;
        }

        std::string content = result.content;
        std::string hash = Hasher::sha256(content);

        auto it = db_map.find(fs_file.path);

        if (it == db_map.end()) {
            FileMetadata meta = fs_file;
            meta.content_hash = hash;

            int file_id = repo.saveFileMetadata(meta);
            repo.saveFileText(file_id, content);
            indexer.indexFile(file_id, content);

            continue;
        }

        FileMetadata& db_file = it->second;
        if (db_file.content_hash != hash) {
            FileMetadata updated = fs_file;
            updated.id = db_file.id;
            updated.content_hash = hash;
            repo.saveFileMetadata(updated);

            repo.deleteIndexForFile(db_file.id);
            repo.saveFileText(db_file.id, content);
            indexer.indexFile(db_file.id, content);
            db_file.content_hash = hash;
        }
    }

    for (const auto& db_file : db_files) {
        if (seen.count(db_file.path) == 0) {
            repo.deleteFileByPath(db_file.path);
        }
    }

    repo.commitTransaction();
}
