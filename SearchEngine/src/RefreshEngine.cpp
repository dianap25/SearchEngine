//Alesia Filinkova
//Diana Pelin

#include "RefreshEngine.h"

#include "Scanner.h"
#include "Extractor.h"
#include "Indexer.h"
#include "Repository.h"

void RefreshEngine::rebuild(const std::string& rootPath, Database& db) {
    Scanner scanner;
    Extractor extractor;

    Repository repo(db.connection());
    Indexer indexer(repo);

    repo.beginTransaction();
    auto files = scanner.scan(rootPath);

    for (const auto& file : files) {
        int fileId = repo.saveFileMetadata(file);
        repo.deleteIndexForFile(fileId);
        std::string content = extractor.extract(file.path);
        repo.saveFileText(fileId, content);
        indexer.indexFile(fileId, content);
    }

    repo.commitTransaction();
}