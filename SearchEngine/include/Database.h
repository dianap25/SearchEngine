// Authors: Alesia Filinkova, Diana Pelin
// Description: Thin RAII wrapper around an SQLite connection. Owns
// the sqlite3 handle, exposes a connection() accessor for Repository
// and runs the schema bootstrap (including the content_hash
// migration) used by index/refresh.

#pragma once

#include <string>

struct sqlite3;

/**
 * @brief Owns the SQLite connection and creates/migrates the schema.
 *
 * The class is non-copyable so the underlying connection has a single
 * owner. Move construction/assignment is defaulted so a Database can
 * be returned from factory functions.
 */
class Database {
public:
    Database() = default;
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;

    /**
     * @brief Open an SQLite database file (or ":memory:" for tests).
     * @param path File path or special SQLite URI.
     * @return true when the connection was opened successfully.
     */
    bool open(const std::string& path);

    /**
     * @brief Create the schema and apply any pending migrations.
     * @return true when the schema is ready to use.
     */
    bool initializeSchema();

    /**
     * @brief Raw connection used by Repository.
     * @return Pointer owned by this Database; do not free.
     */
    sqlite3* connection();

private:
    bool executeSql(const std::string& sql);

    sqlite3* db_ = nullptr;
};
