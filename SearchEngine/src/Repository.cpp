//Alesia Filinkova
//Diana Pelin

#include "Repository.h"

#include <sqlite3.h>

#include <ctime>
#include <iostream>

Repository::Repository(sqlite3* db)
    : db_(db) {
}

bool Repository::beginTransaction() {
    return executeSql("BEGIN TRANSACTION;");
}

bool Repository::commitTransaction() {
    return executeSql("COMMIT;");
}

bool Repository::rollbackTransaction() {
    return executeSql("ROLLBACK;");
}

bool Repository::executeSql(const std::string& sql) {
    char* errorMessage = nullptr;

    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: "
                  << (errorMessage != nullptr ? errorMessage : "unknown error")
                  << "\n";

        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

int Repository::saveFileMetadata(const FileMetadata& metadata) {
    const char* sql = R"(
        INSERT INTO files(path, name, extension, size, modified_time, indexed_at)
        VALUES (?, ?, ?, ?, ?, ?)
        ON CONFLICT(path) DO UPDATE SET
            name = excluded.name,
            extension = excluded.extension,
            size = excluded.size,
            modified_time = excluded.modified_time,
            indexed_at = excluded.indexed_at
        RETURNING id;
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare saveFileMetadata statement\n";
        return -1;
    }

    const std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));

    sqlite3_bind_text(statement, 1, metadata.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, metadata.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, metadata.extension.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 4, static_cast<sqlite3_int64>(metadata.size));
    sqlite3_bind_int64(statement, 5, static_cast<sqlite3_int64>(metadata.modifiedTime));
    sqlite3_bind_int64(statement, 6, static_cast<sqlite3_int64>(now));

    int fileId = -1;

    if (sqlite3_step(statement) == SQLITE_ROW) {
        fileId = sqlite3_column_int(statement, 0);
    } else {
        std::cerr << "Failed to save file metadata: "
                  << sqlite3_errmsg(db_)
                  << "\n";
    }

    sqlite3_finalize(statement);
    return fileId;
}

bool Repository::saveFileText(int fileId, const std::string& content) {
    const char* sql = R"(
        INSERT INTO file_texts(file_id, content)
        VALUES (?, ?)
        ON CONFLICT(file_id) DO UPDATE SET
            content = excluded.content;
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare saveFileText statement\n";
        return false;
    }

    sqlite3_bind_int(statement, 1, fileId);
    sqlite3_bind_text(statement, 2, content.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(statement) == SQLITE_DONE;

    if (!success) {
        std::cerr << "Failed to save file text: "
                  << sqlite3_errmsg(db_)
                  << "\n";
    }

    sqlite3_finalize(statement);
    return success;
}

int Repository::findOrCreateTerm(const std::string& term) {
    const char* insertSql = R"(
        INSERT INTO terms(term)
        VALUES (?)
        ON CONFLICT(term) DO NOTHING;
    )";

    sqlite3_stmt* insertStatement = nullptr;

    if (sqlite3_prepare_v2(db_, insertSql, -1, &insertStatement, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(insertStatement, 1, term.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(insertStatement);
    sqlite3_finalize(insertStatement);

    const char* selectSql = "SELECT id FROM terms WHERE term = ?;";

    sqlite3_stmt* selectStatement = nullptr;

    if (sqlite3_prepare_v2(db_, selectSql, -1, &selectStatement, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(selectStatement, 1, term.c_str(), -1, SQLITE_TRANSIENT);

    int termId = -1;

    if (sqlite3_step(selectStatement) == SQLITE_ROW) {
        termId = sqlite3_column_int(selectStatement, 0);
    }

    sqlite3_finalize(selectStatement);
    return termId;
}

bool Repository::saveTermPosition(int fileId, const std::string& term, int position) {
    int termId = findOrCreateTerm(term);

    if (termId < 0) {
        return false;
    }

    const char* sql = R"(
        INSERT INTO postings(term_id, file_id, position)
        VALUES (?, ?, ?);
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(statement, 1, termId);
    sqlite3_bind_int(statement, 2, fileId);
    sqlite3_bind_int(statement, 3, position);

    bool success = sqlite3_step(statement) == SQLITE_DONE;

    if (!success) {
        std::cerr << "Failed to save term position: "
                  << sqlite3_errmsg(db_)
                  << "\n";
    }

    sqlite3_finalize(statement);
    return success;
}

std::optional<FileMetadata> Repository::findByPath(const std::string& path) {
    const char* sql = R"(
        SELECT path, name, extension, size, modified_time
        FROM files
        WHERE path = ?;
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, path.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<FileMetadata> result = std::nullopt;

    if (sqlite3_step(statement) == SQLITE_ROW) {
        FileMetadata metadata;
        metadata.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        metadata.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        metadata.extension = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        metadata.size = static_cast<std::uintmax_t>(sqlite3_column_int64(statement, 3));
        metadata.modifiedTime = static_cast<std::int64_t>(sqlite3_column_int64(statement, 4));

        result = metadata;
    }

    sqlite3_finalize(statement);
    return result;
}

std::optional<std::string> Repository::findTextByPath(const std::string& path) {
    const char* sql = R"(
        SELECT ft.content
        FROM files f
        JOIN file_texts ft ON ft.file_id = f.id
        WHERE f.path = ?;
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare findTextByPath statement: "
                  << sqlite3_errmsg(db_)
                  << "\n";
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, path.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<std::string> result = std::nullopt;

    if (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(statement, 0);

        if (text != nullptr) {
            result = reinterpret_cast<const char*>(text);
        }
    }

    sqlite3_finalize(statement);
    return result;
}

std::vector<FileMetadata> Repository::findAllFiles() {
    const char* sql = R"(
        SELECT path, name, extension, size, modified_time
        FROM files;
    )";

    sqlite3_stmt* statement = nullptr;
    std::vector<FileMetadata> files;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return files;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        FileMetadata metadata;
        metadata.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        metadata.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        metadata.extension = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        metadata.size = static_cast<std::uintmax_t>(sqlite3_column_int64(statement, 3));
        metadata.modifiedTime = static_cast<std::int64_t>(sqlite3_column_int64(statement, 4));

        files.push_back(metadata);
    }

    sqlite3_finalize(statement);
    return files;
}

bool Repository::deleteFileByPath(const std::string& path) {
    const char* sql = "DELETE FROM files WHERE path = ?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, path.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(statement) == SQLITE_DONE;

    sqlite3_finalize(statement);
    return success;
}

bool Repository::deleteIndexForFile(int fileId) {
    const char* sql = "DELETE FROM postings WHERE file_id = ?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(statement, 1, fileId);

    bool success = sqlite3_step(statement) == SQLITE_DONE;

    sqlite3_finalize(statement);
    return success;
}

std::vector<SearchResult> Repository::searchByName(const std::string& phrase) {
    const char* sql = R"(
        SELECT path, name
        FROM files
        WHERE LOWER(name) LIKE LOWER(?)
        ORDER BY name ASC;
    )";

    sqlite3_stmt* statement = nullptr;
    std::vector<SearchResult> results;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return results;
    }

    std::string pattern = "%" + phrase + "%";
    sqlite3_bind_text(statement, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        SearchResult result;
        result.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        result.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        result.occurrences = 1;

        results.push_back(result);
    }

    sqlite3_finalize(statement);
    return results;
}

std::vector<SearchResult> Repository::searchByContent(const std::string& term) {
    const char* sql = R"(
        SELECT f.path, f.name, COUNT(p.position) AS occurrences
        FROM terms t
        JOIN postings p ON p.term_id = t.id
        JOIN files f ON f.id = p.file_id
        WHERE t.term = ?
        GROUP BY f.id, f.path, f.name
        ORDER BY occurrences DESC, f.name ASC;
    )";

    sqlite3_stmt* statement = nullptr;
    std::vector<SearchResult> results;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return results;
    }

    sqlite3_bind_text(statement, 1, term.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        SearchResult result;
        result.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        result.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        result.occurrences = sqlite3_column_int(statement, 2);

        results.push_back(result);
    }

    sqlite3_finalize(statement);
    return results;
}