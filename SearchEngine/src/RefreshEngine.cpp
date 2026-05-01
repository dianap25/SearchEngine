//Alesia Filinkova
//Diana Pelin

#include "RefreshEngine.h"

#include "Scanner.h"
#include "Extractor.h"
#include "Indexer.h"
#include "Repository.h"
#include "Hasher.h"

#include <unordered_map>
#include <unordered_set>
#include <iostream>

void RefreshEngine::refresh(const std::string& rootPath, Database& db) {
    Scanner scanner;
    Extractor extractor;

    Repository repo(db.connection());
    Indexer indexer(repo);

    repo.beginTransaction();

    auto fsFiles = scanner.scan(rootPath);
    auto dbFiles = repo.findAllFiles();

    std::unordered_map<std::string, FileMetadata> dbMap;
    for (const auto& f : dbFiles) {
        dbMap[f.path] = f;
    }

    std::unordered_set<std::string> seen;

    for (const auto& fsFile : fsFiles) {
        seen.insert(fsFile.path);
        ExtractResult result = extractor.extract(fsFile.path);
        if (!result.success) {
            std::cerr << "[SKIP] " << fsFile.path
                      << " -> " << result.errorMessage << "\n";
            continue;
        }

        std::string content = result.content;
        std::string hash = Hasher::sha256(content);

        auto it = dbMap.find(fsFile.path);

        if (it == dbMap.end()) {
            FileMetadata meta = fsFile;
            meta.contentHash = hash;

            int fileId = repo.saveFileMetadata(meta);
            repo.saveFileText(fileId, content);
            indexer.indexFile(fileId, content);

            continue;
        }

        FileMetadata& dbFile = it->second;
        if (dbFile.contentHash != hash) {
            repo.deleteIndexForFile(dbFile.id);
            repo.saveFileText(dbFile.id, content);
            indexer.indexFile(dbFile.id, content);
            dbFile.contentHash = hash;
        }
    }

    for (const auto& dbFile : dbFiles) {
        if (!seen.count(dbFile.path)) {
            repo.deleteFileByPath(dbFile.path);
        }
    }

    repo.commitTransaction();
}