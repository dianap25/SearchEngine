// Authors: Alesia Filinkova, Diana Pelin
// Description: Database access layer over the index schema.

#pragma once

#include "FileMetadata.h"
#include "SearchResult.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

struct sqlite3;

/**
 * @brief Persistence layer for the files, file_texts, terms and
 *        postings tables.
 *
 * Repository does not own the connection; the caller (usually the
 * Database owner) guarantees that the sqlite3* handle outlives the
 * Repository object.
 */
class Repository {
public:
    explicit Repository(sqlite3* db);

    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = default;
    Repository& operator=(Repository&&) = default;

    /** @brief Begins a SQL transaction. @return true on success. */
    bool beginTransaction();
    /** @brief Commits the open SQL transaction. */
    bool commitTransaction();
    /** @brief Rolls back the open SQL transaction. */
    bool rollbackTransaction();

    /**
     * @brief Inserts or updates a row in the files table.
     * @param metadata File metadata (id may be zero when inserting a
     *                 brand new record).
     * @return Numeric row id, or -1 on failure.
     */
    int saveFileMetadata(const FileMetadata& metadata);

    /**
     * @brief Replaces the cached extracted text for a file.
     * @param file_id Database id of the file.
     * @param content Extracted text to persist.
     */
    bool saveFileText(int file_id, const std::string& content);

    /**
     * @brief Inserts every posting for a single file using prepared
     *        statements and an in-memory term cache.
     *
     * The term-insert, term-select and posting-insert statements are
     * prepared once; the method then iterates over @p tokens, binding,
     * stepping and resetting each statement. Repeated terms in the
     * same file are resolved through the in-memory cache, which
     * removes the extra round-trip to the terms table.
     *
     * @param file_id Database id of the file.
     * @param tokens Pairs of (normalized term, position) in document
     *               order.
     * @return true if every posting was persisted successfully.
     */
    bool saveTermPositionsBatch(
        int file_id,
        const std::vector<std::pair<std::string, int>>& tokens
    );

    /** @brief Looks up file metadata by exact path. */
    std::optional<FileMetadata> findByPath(const std::string& path);
    /** @brief Looks up the extracted text by exact path. */
    std::optional<std::string> findTextByPath(const std::string& path);
    /** @brief Returns metadata of every currently indexed file. */
    std::vector<FileMetadata> findAllFiles();

    /** @brief Deletes a row from the files table (cascades to postings/file_texts). */
    bool deleteFileByPath(const std::string& path);
    /** @brief Deletes every posting that belongs to a single file. */
    bool deleteIndexForFile(int file_id);

    /** @brief Searches files by a fragment of their name (case-insensitive). */
    std::vector<SearchResult> searchByName(const std::string& phrase);
    /** @brief Searches files by an exact, normalized term in their content. */
    std::vector<SearchResult> searchByContent(const std::string& term);

private:
    bool executeSql(const std::string& sql);

    sqlite3* db_;
};
