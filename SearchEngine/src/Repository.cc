// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of the Repository class. Encapsulates
// every SQL statement used by the engine: file metadata UPSERT,
// extracted-text caching, term/posting writes and the lookup queries
// that drive search-name and search-content.

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
    char* error_message = nullptr;

    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_message);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: "
                  << (error_message != nullptr ? error_message : "unknown error")
                  << "\n";

        sqlite3_free(error_message);
        return false;
    }

    return true;
}

int Repository::saveFileMetadata(const FileMetadata& metadata) {
    const char* sql = R"(
        INSERT INTO files(path, name, extension, size, modified_time, indexed_at, content_hash)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(path) DO UPDATE SET
            name = excluded.name,
            extension = excluded.extension,
            size = excluded.size,
            modified_time = excluded.modified_time,
            indexed_at = excluded.indexed_at,
            content_hash = excluded.content_hash
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
    sqlite3_bind_int64(statement, 5, static_cast<sqlite3_int64>(metadata.modified_time));
    sqlite3_bind_int64(statement, 6, static_cast<sqlite3_int64>(now));
    sqlite3_bind_text(statement, 7, metadata.content_hash.c_str(), -1, SQLITE_TRANSIENT);

    int file_id = -1;

    if (sqlite3_step(statement) == SQLITE_ROW) {
        file_id = sqlite3_column_int(statement, 0);
    } else {
        std::cerr << "Failed to save file metadata: "
                  << sqlite3_errmsg(db_)
                  << "\n";
    }

    sqlite3_finalize(statement);
    return file_id;
}

bool Repository::saveFileText(int file_id, const std::string& content) {
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

    sqlite3_bind_int(statement, 1, file_id);
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
    const char* insert_sql = R"(
        INSERT INTO terms(term)
        VALUES (?)
        ON CONFLICT(term) DO NOTHING;
    )";

    sqlite3_stmt* insert_statement = nullptr;

    if (sqlite3_prepare_v2(db_, insert_sql, -1, &insert_statement, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(insert_statement, 1, term.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(insert_statement);
    sqlite3_finalize(insert_statement);

    const char* select_sql = "SELECT id FROM terms WHERE term = ?;";

    sqlite3_stmt* select_statement = nullptr;

    if (sqlite3_prepare_v2(db_, select_sql, -1, &select_statement, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(select_statement, 1, term.c_str(), -1, SQLITE_TRANSIENT);

    int term_id = -1;

    if (sqlite3_step(select_statement) == SQLITE_ROW) {
        term_id = sqlite3_column_int(select_statement, 0);
    }

    sqlite3_finalize(select_statement);
    return term_id;
}

bool Repository::saveTermPosition(int file_id, const std::string& term, int position) {
    int term_id = findOrCreateTerm(term);

    if (term_id < 0) {
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

    sqlite3_bind_int(statement, 1, term_id);
    sqlite3_bind_int(statement, 2, file_id);
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
        SELECT id, path, name, extension, size, modified_time, content_hash
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
        metadata.id = sqlite3_column_int(statement, 0);
        metadata.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        metadata.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        metadata.extension = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        metadata.size = static_cast<std::uintmax_t>(sqlite3_column_int64(statement, 4));
        metadata.modified_time = static_cast<std::int64_t>(sqlite3_column_int64(statement, 5));
        const unsigned char* hash_text = sqlite3_column_text(statement, 6);
        if (hash_text != nullptr) {
            metadata.content_hash = reinterpret_cast<const char*>(hash_text);
        }

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
        SELECT id, path, name, extension, size, modified_time, content_hash
        FROM files;
    )";

    sqlite3_stmt* statement = nullptr;
    std::vector<FileMetadata> files;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return files;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        FileMetadata metadata;
        metadata.id = sqlite3_column_int(statement, 0);
        metadata.path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        metadata.name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        metadata.extension = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        metadata.size = static_cast<std::uintmax_t>(sqlite3_column_int64(statement, 4));
        metadata.modified_time = static_cast<std::int64_t>(sqlite3_column_int64(statement, 5));
        const unsigned char* hash_text = sqlite3_column_text(statement, 6);
        if (hash_text != nullptr) {
            metadata.content_hash = reinterpret_cast<const char*>(hash_text);
        }

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

bool Repository::deleteIndexForFile(int file_id) {
    const char* sql = "DELETE FROM postings WHERE file_id = ?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(statement, 1, file_id);

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
