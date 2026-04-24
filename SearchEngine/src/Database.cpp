//Alesia Filinkova
//Diana Pelin

#include "Database.h"

#include <sqlite3.h>
#include <iostream>

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::open(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database\n";
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

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE,
            content TEXT
        );
    )";

    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to initialize schema: "
                  << (errorMessage != nullptr ? errorMessage : "unknown error")
                  << "\n";
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}