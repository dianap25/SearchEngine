// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Testy jednostkowe klasy Database. Pokrywają open(),
// inicjalizację schematu i weryfikują, że wszystkie wymagane tabele
// oraz indeksy zostały utworzone przez initializeSchema().

#include <gtest/gtest.h>

#include "Database.h"

#include <cstdio>
#include <sqlite3.h>
#include <string>

namespace {

bool tableExists(sqlite3* db, const std::string& table_name) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

bool indexExists(sqlite3* db, const std::string& index_name) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='index' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, index_name.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

} // namespace

TEST(DatabaseTest, OpensDatabaseFile) {
    const std::string db_path = "open_database_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        EXPECT_TRUE(database.open(db_path));
    }

    std::remove(db_path.c_str());
}

TEST(DatabaseTest, InitializesSchemaAfterOpen) {
    const std::string db_path = "initialize_schema_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(db_path));
        EXPECT_TRUE(database.initializeSchema());
    }

    std::remove(db_path.c_str());
}

TEST(DatabaseTest, FailsToInitializeSchemaWithoutOpen) {
    Database database;

    EXPECT_FALSE(database.initializeSchema());
}

TEST(DatabaseTest, CreatesRequiredTablesAndIndexes) {
    const std::string db_path = "schema_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(tableExists(raw_db, "files"));
    EXPECT_TRUE(tableExists(raw_db, "terms"));
    EXPECT_TRUE(tableExists(raw_db, "postings"));
    EXPECT_TRUE(tableExists(raw_db, "file_texts"));

    EXPECT_TRUE(indexExists(raw_db, "idx_files_path"));
    EXPECT_TRUE(indexExists(raw_db, "idx_files_name"));
    EXPECT_TRUE(indexExists(raw_db, "idx_terms_term"));
    EXPECT_TRUE(indexExists(raw_db, "idx_postings_term_file"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}
