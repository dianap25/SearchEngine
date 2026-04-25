//Alesia Filinkova
//Diana Pelin

#pragma once

#include "FileMetadata.h"
#include "SearchResult.h"

#include <optional>
#include <string>
#include <vector>

struct sqlite3;

class Repository {
public:
    explicit Repository(sqlite3* db);

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    int saveFileMetadata(const FileMetadata& metadata);
    bool saveFileText(int fileId, const std::string& content);
    bool saveTermPosition(int fileId, const std::string& term, int position);

    std::optional<FileMetadata> findByPath(const std::string& path);
    std::vector<FileMetadata> findAllFiles();

    bool deleteFileByPath(const std::string& path);
    bool deleteIndexForFile(int fileId);

    std::vector<SearchResult> searchByName(const std::string& phrase);
    std::vector<SearchResult> searchByContent(const std::string& term);

private:
    bool executeSql(const std::string& sql);
    int findOrCreateTerm(const std::string& term);

    sqlite3* db_;
};