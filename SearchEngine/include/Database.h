// Authors: Alesia Filinkova, Diana Pelin
// Description: Thin RAII wrapper around an SQLite connection. Owns
// the sqlite3 handle through std::unique_ptr with a custom deleter,
// exposes a connection() accessor for Repository, and runs the
// schema bootstrap (including the content_hash migration) used by
// index/refresh.

#pragma once

#include <memory>
#include <string>

struct sqlite3;

/**
 * @brief Owns the SQLite connection and creates/migrates the schema.
 *
 * The connection is held in a std::unique_ptr with a custom deleter,
 * so closing happens automatically when the Database goes out of
 * scope. The class is explicitly non-copyable; move construction and
 * move assignment are defaulted so a Database can be returned from
 * factory functions.
 */
class Database {
public:
    Database();

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
    using SqliteHandle = std::unique_ptr<sqlite3, void (*)(sqlite3*)>;

    bool executeSql(const std::string& sql);
    static SqliteHandle makeHandle(sqlite3* raw = nullptr);

    SqliteHandle db_;
};
