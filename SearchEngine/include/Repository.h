// Authors: Alesia Filinkova, Diana Pelin
// Description: SQL-facing interface around the index database. All
// reads and writes against files, file_texts, terms and postings
// tables happen here so the rest of the engine can stay free of raw
// SQL.

#pragma once

#include "FileMetadata.h"
#include "SearchResult.h"

#include <optional>
#include <string>
#include <vector>

struct sqlite3;

/**
 * @brief Persistence layer for files, file_texts, terms and postings.
 *
 * The instance does not own the connection; the caller (typically a
 * Database owner) keeps the sqlite3* alive for the lifetime of the
 * Repository.
 */
class Repository {
public:
    explicit Repository(sqlite3* db);

    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = default;
    Repository& operator=(Repository&&) = default;

    /** @brief Begin a SQL transaction. @return true on success. */
    bool beginTransaction();
    /** @brief Commit the open SQL transaction. */
    bool commitTransaction();
    /** @brief Roll back the open SQL transaction. */
    bool rollbackTransaction();

    /**
     * @brief Insert or update a row in the files table.
     * @param metadata Metadata for the file (id may be zero on insert).
     * @return Numeric id of the row, or -1 on failure.
     */
    int saveFileMetadata(const FileMetadata& metadata);

    /**
     * @brief Replace the cached extracted text for a file.
     * @param file_id Database id of the file.
     * @param content Extracted text to store.
     */
    bool saveFileText(int file_id, const std::string& content);

    /**
     * @brief Append a single (term, position) pair to postings.
     * @param file_id Database id of the file.
     * @param term Normalized term string.
     * @param position Token position within the file.
     */
    bool saveTermPosition(int file_id, const std::string& term, int position);

    /** @brief Look up file metadata by exact path. */
    std::optional<FileMetadata> findByPath(const std::string& path);
    /** @brief Look up extracted text by exact path. */
    std::optional<std::string> findTextByPath(const std::string& path);
    /** @brief Return metadata for every file currently indexed. */
    std::vector<FileMetadata> findAllFiles();

    /** @brief Remove a file row (cascades to postings/file_texts). */
    bool deleteFileByPath(const std::string& path);
    /** @brief Drop all postings belonging to a single file. */
    bool deleteIndexForFile(int file_id);

    /** @brief Search files by case-insensitive substring of name. */
    std::vector<SearchResult> searchByName(const std::string& phrase);
    /** @brief Search files by exact normalized term in their content. */
    std::vector<SearchResult> searchByContent(const std::string& term);

private:
    bool executeSql(const std::string& sql);
    int findOrCreateTerm(const std::string& term);

    sqlite3* db_;
};
