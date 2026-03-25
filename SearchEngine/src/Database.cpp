#include "Database.h"

#include <sqlite3.h>
#include <iostream>

bool Database::open(const std::string& path) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(path.c_str(), &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database\n";
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return false;
    }

    sqlite3_close(db);
    return true;
}