// Authors: Alesia Filinkova, Diana Pelin
// Description: RAII wrapper around a SQLite connection. Owns the
// sqlite3 handle through std::unique_ptr with a custom deleter,
// exposes connection() for the Repository class, and creates and
// migrates the schema used by the index/refresh commands.

#pragma once

#include <memory>
#include <string>

struct sqlite3;

/**
 * @brief Owns a SQLite connection and creates/migrates the schema.
 *
 * The connection is held in std::unique_ptr with a custom deleter, so
 * the database is closed automatically when the Database object goes
 * out of scope. The class is explicitly non-copyable; move
 * construction and move assignment are defaulted, so Database can be
 * returned from factory functions.
 */
class Database {
public:
    Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;

    /**
     * @brief Opens a SQLite database file (or ":memory:" in tests).
     * @param path File path or a SQLite-specific URI.
     * @return true if the connection was opened successfully.
     */
    bool open(const std::string& path);

    /**
     * @brief Creates the database schema and runs pending migrations.
     * @return true if the schema is ready for use.
     */
    bool initializeSchema();

    /**
     * @brief Raw connection handle used by Repository.
     * @return Pointer owned by this Database object; the caller must
     *         not close it.
     */
    sqlite3* connection();

private:
    using SqliteHandle = std::unique_ptr<sqlite3, void (*)(sqlite3*)>;

    bool executeSql(const std::string& sql);
    static SqliteHandle makeHandle(sqlite3* raw = nullptr);

    SqliteHandle db_;
};
