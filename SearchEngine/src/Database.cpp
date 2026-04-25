//Alesia Filinkova
//Diana Pelin

#include "Database.h"

#include <sqlite3.h>

#include <iostream>
#include <string>

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::open(const std::string& path) {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    int rc = sqlite3_open(path.c_str(), &db_);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: "
                  << (db_ != nullptr ? sqlite3_errmsg(db_) : "unknown error")
                  << "\n";

        if (db_ != nullptr) {
            sqlite3_close(db_);
            db_ = nullptr;
        }

        return false;
    }

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
            indexed_at INTEGER NOT NULL
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

    return executeSql(sql);
}

bool Database::executeSql(const std::string& sql) {
    char* errorMessage = nullptr;

    int rc = sqlite3_exec(
        db_,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: "
                  << (errorMessage != nullptr ? errorMessage : "unknown error")
                  << "\n";

        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

sqlite3* Database::connection() {
    return db_;
}