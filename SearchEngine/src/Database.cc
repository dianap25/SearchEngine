// Autorzy: Alesia Filinkova, Diana Pelin


#include "Database.h"

#include <sqlite3.h>

#include <iostream>
#include <string>

namespace {

void closeSqlite(sqlite3* handle) {
    if (handle != nullptr) {
        sqlite3_close(handle);
    }
}

} // namespace

Database::SqliteHandle Database::makeHandle(sqlite3* raw) {
    return SqliteHandle(raw, &closeSqlite);
}

Database::Database()
    : db_(makeHandle()) {
}

bool Database::open(const std::string& path) {
    db_.reset();

    sqlite3* raw = nullptr;
    int rc = sqlite3_open(path.c_str(), &raw);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: "
                  << (raw != nullptr ? sqlite3_errmsg(raw) : "unknown error")
                  << "\n";

        if (raw != nullptr) {
            sqlite3_close(raw);
        }

        return false;
    }

    db_ = makeHandle(raw);
    return true;
}

bool Database::initializeSchema() {
    if (db_ == nullptr) {
        std::cerr << "Database is not open\n";
        return false;
    }

    const std::string sql = R"(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            extension TEXT,
            size INTEGER NOT NULL,
            modified_time INTEGER NOT NULL,
            indexed_at INTEGER NOT NULL,
            content_hash TEXT NOT NULL DEFAULT ''
        );

        CREATE TABLE IF NOT EXISTS terms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            term TEXT NOT NULL UNIQUE
        );

        CREATE TABLE IF NOT EXISTS postings (
            term_id INTEGER NOT NULL,
            file_id INTEGER NOT NULL,
            position INTEGER NOT NULL,
            FOREIGN KEY(term_id) REFERENCES terms(id) ON DELETE CASCADE,
            FOREIGN KEY(file_id) REFERENCES files(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS file_texts (
            file_id INTEGER PRIMARY KEY,
            content TEXT NOT NULL,
            FOREIGN KEY(file_id) REFERENCES files(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_files_path
            ON files(path);

        CREATE INDEX IF NOT EXISTS idx_files_name
            ON files(name);

        CREATE INDEX IF NOT EXISTS idx_terms_term
            ON terms(term);

        CREATE INDEX IF NOT EXISTS idx_postings_term_file
            ON postings(term_id, file_id);
    )";

    if (!executeSql(sql)) {
        return false;
    }

    // Lekka migracja dla starszych baz, które powstały zanim
    // pojawiła się kolumna content_hash. Gdy kolumna już istnieje,
    // SQLite zwraca SQLITE_ERROR z komunikatem "duplicate column
    // name"; ten przypadek świadomie ignorujemy.
    char* alter_error = nullptr;
    int alter_rc = sqlite3_exec(
        db_.get(),
        "ALTER TABLE files ADD COLUMN content_hash TEXT NOT NULL DEFAULT '';",
        nullptr,
        nullptr,
        &alter_error
    );

    if (alter_rc != SQLITE_OK) {
        const std::string message = alter_error != nullptr ? alter_error : "";
        sqlite3_free(alter_error);

        if (message.find("duplicate column") == std::string::npos) {
            std::cerr << "Schema migration failed: " << message << "\n";
            return false;
        }
    }

    return true;
}

bool Database::executeSql(const std::string& sql) {
    char* error_message = nullptr;

    int rc = sqlite3_exec(
        db_.get(),
        sql.c_str(),
        nullptr,
        nullptr,
        &error_message
    );

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: "
                  << (error_message != nullptr ? error_message : "unknown error")
                  << "\n";

        sqlite3_free(error_message);
        return false;
    }

    return true;
}

sqlite3* Database::connection() {
    return db_.get();
}
